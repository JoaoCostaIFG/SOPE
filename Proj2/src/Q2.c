#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "include/init.h"
#include "include/logs.h"
#include "include/packet.h"
#include "include/utls.h"
#include "include/vector.h"

#define NO_SPOT       -1
#define BATHROOMEMPTY (void*)0
#define BATHROOMFULL  (void*)1

static pthread_mutex_t bathroom_mutex = PTHREAD_MUTEX_INITIALIZER;
static vector* bathroom               = NULL;
static prog_prop prog_props;
static int request_fd;
static int is_open;
static sem_t bathroom_sem;
static sem_t threads_sem;

static void
sig_handler(int signo)
{
  switch (signo) {
    case SIGALRM:
      is_open = 0;
      break;
    case SIGINT:
      exit(EXIT_FAILURE);
      break;
    default:
      break;
  }
}

static void
setup_signal_handler(void)
{
  struct sigaction sa;
  sa.sa_handler = &sig_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART; // restart calls instead of fail w/ EINTR
  if (sigaction(SIGALRM, &sa, NULL) == -1 ||
      sigaction(SIGINT, &sa, NULL) == -1) {
    fprintf(stderr, "Signal handler setup failed.\n");
    exit(EXIT_FAILURE);
  }

  // alarm signal
  is_open = 1; // alarm control variable
  alarm(prog_props.nsecs);
}

static int
choose_bathroom_spot(void)
{
  int spot = NO_SPOT;

  if (prog_props.nplaces != EMPTY_FIELD)
    sem_wait(&bathroom_sem);
  pthread_mutex_lock(&bathroom_mutex);

  for (size_t i = 0; i < bathroom->end; ++i) {
    if (vector_at(bathroom, i) == BATHROOMEMPTY) {
      vector_set(bathroom, i, BATHROOMFULL);
      spot = i;
      break;
    }
  }

  if (spot == NO_SPOT && prog_props.nplaces == EMPTY_FIELD) {
    vector_push_back(bathroom, BATHROOMFULL);
    spot = bathroom->end - 1;
  }

  pthread_mutex_unlock(&bathroom_mutex);
  return spot;
}

static void
free_bathroom_spot(int i)
{
  pthread_mutex_lock(&bathroom_mutex);
  vector_set(bathroom, i, BATHROOMEMPTY);
  pthread_mutex_unlock(&bathroom_mutex);
}

static void
prepare_answer(packet* request, packet* answer)
{
  answer->i   = request->i;
  answer->pid = getpid();
  answer->tid = pthread_self();

  if (is_open) {
    answer->dur = request->dur;
    if ((answer->pl = choose_bathroom_spot()) == NO_SPOT)
      answer->pl = EMPTY_FIELD;
  }
  else { // Bathroom is closing
    answer->dur = EMPTY_FIELD;
    answer->pl  = EMPTY_FIELD;
  }

  RECVD_LOG(answer);
}

static void
cleanup_request_handler(void* arg)
{
  free(arg);
  if (prog_props.nthreads != EMPTY_FIELD)
    sem_post(&threads_sem);
}

static void*
request_handler(void* arg)
{
  pthread_cleanup_push(cleanup_request_handler, arg);

  packet* request = (packet*)arg;

  char answer_fifo[80];
  packet answer;
  prepare_answer(request, &answer);
  sprintf(answer_fifo, "%s/%d.%ld", FIFO_DIR, request->pid, request->tid);

  // open com with client
  int answer_fd;
  if ((answer_fd = open(answer_fifo, O_WRONLY)) < 0) { // open answer fifo
    GAVUP_LOG(&answer);
    pthread_exit(NULL);
  }

  int write_ans = write_sigsafe(answer_fd, &answer, sizeof(packet));
  close(answer_fd);

  if (answer.pl == EMPTY_FIELD) {
    // didn't alloc bathroom
    LATE2_LOG(&answer);
  }
  else {
    if (write_ans < 0) {
      GAVUP_LOG(&answer);
    }
    else {
      ENTER_LOG(&answer);
      // Isto asegura que o sleep e retomado caso seja interrompido por um sinal
      // (nao assegurado por SA_RESTART).
      long sleep_dur = answer.dur;
      do {
        sleep_dur = sleep(sleep_dur);
      } while (sleep_dur > 0);
      TIMUP_LOG(&answer);
    }

    free_bathroom_spot(answer.pl);
    if (prog_props.nplaces != EMPTY_FIELD)
      sem_post(&bathroom_sem);
  }

  pthread_exit(NULL);
  pthread_cleanup_pop(0);
}

static void
clean_up_server(void)
{
  alarm(0); // cancel alarms

  if (close(request_fd))
    perror(prog_props.fifoname);
  if (unlink(prog_props.fifoname))
    perror(prog_props.fifoname);

  free_vector(bathroom);
  if (pthread_mutex_destroy(&bathroom_mutex))
    fprintf(stderr, "Mutex destruction error.");

  if (prog_props.nthreads != EMPTY_FIELD)
    sem_destroy(&threads_sem);
  if (prog_props.nplaces != EMPTY_FIELD)
    sem_destroy(&bathroom_sem);
}

static void
init_server_res(vector** threads)
{
  /* public resources */
  // create public communication channel (FIFO)
  if ((request_fd = fifo_mk_open(prog_props.fifoname, 0660, O_RDONLY)) < 0) {
    fprintf(stderr, "Public communication FIFO creation failed.\n");
    exit(EXIT_FAILURE);
  }

  /* private resources */
  // internal use semaphores
  if (prog_props.nthreads != EMPTY_FIELD &&
      sem_init(&threads_sem, 0, prog_props.nthreads) < 0) {
    perror("Couldn't create thread limiting semaphore.\n");
    exit(EXIT_FAILURE);
  }
  if (prog_props.nplaces != EMPTY_FIELD &&
      sem_init(&bathroom_sem, 0, prog_props.nplaces) < 0) {
    perror("Couldn't create bathroom places limiting semaphore.\n");
    exit(EXIT_FAILURE);
  }

  // state storage resources
  if ((bathroom = new_vector()) == NULL) {
    fprintf(stderr, "Bathroom malloc failed.\n");
    exit(EXIT_FAILURE);
  }
  // create one postion per bathroom stall (if given)
  if (prog_props.nplaces != EMPTY_FIELD) {
    for (size_t i = 0; i < (size_t)prog_props.nplaces; ++i)
      vector_push_back(bathroom, BATHROOMEMPTY);
  }

  if ((*threads = new_vector()) == NULL) {
    fprintf(stderr, "Threads malloc failed.\n");
    exit(EXIT_FAILURE);
  }
}

static void
main_loop(vector** threads, struct timespec* end_time)
{
  /* main loop */
  int read_ans;
  pthread_t* pt;
  packet* request;
  do {
    if ((request = (packet*)malloc(sizeof(packet))) == NULL) {
      fprintf(stderr, "Request packet memory allocation failed.");
      exit(EXIT_FAILURE);
    }

    if ((read_ans = read_sigsafe(request_fd, request, sizeof(packet))) < 0) {
      perror("Server public fifo failed.");
      break;
    }

    if (read_ans == 0) {
      free(request);
      break;
    }
    else {
      if ((pt = (pthread_t*)malloc(sizeof(pthread_t))) == NULL) {
        fprintf(stderr, "Pthread memory allocation failed.");
        exit(EXIT_FAILURE);
      }

      if (prog_props.nthreads != EMPTY_FIELD)
        sem_wait(&threads_sem);
      vector_push_back(*threads, pt);
      if (pthread_create(pt, NULL, request_handler, request) != 0) {
        perror("Failed creating thread. Stopping..");
        break;
      }
    }
  } while (is_open || read_ans > 0);

  // join all threads
  for (size_t i = 0; i < (*threads)->end; ++i) {
    pt = (pthread_t*)vector_at(*threads, i);
    pthread_join(*pt, NULL);
  }
  free_vector_data(*threads);
  free_vector(*threads);
}

int
main(int argc, char* argv[])
{
  /* interpret command line arguments */
  init_q(argc, argv, &prog_props);

  /* set-up server */
  vector* threads;
  init_server_res(
    &threads); // open FIFO, allocate memory and initialize variables
  atexit(clean_up_server);
  struct timespec end_time;
  setup_signal_handler(); // set alarm and fill end_time struct

  main_loop(&threads, &end_time);

  return 0;
}
