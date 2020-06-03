#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "include/init.h"
#include "include/logs.h"
#include "include/packet.h"
#include "include/utls.h"

#define MAX_REQUEST_DUR 3
#define MIN_REQUEST_DUR 1

#define SEC_BETWEEN_REQUESTS 10
#define SEC2MILI             1000
#define MICROS2MS            1000

static prog_prop prog_props;
static int is_open = 1; // server is open
static int request_fd;
static pthread_t* threads = NULL;

static void
sig_handler(int signo)
{
  switch (signo) {
    case SIGINT:
      exit(EXIT_FAILURE);
      break;
    case SIGPIPE:
      is_open = 0;
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

  if (sigaction(SIGINT, &sa, NULL) == -1 ||
      sigaction(SIGPIPE, &sa, NULL) == -1) {
    fprintf(stderr, "Signal handler setup failed.\n");
    exit(EXIT_FAILURE);
  }
}

static void
gen_request(packet* request, int i)
{
  request->i   = i;
  request->pid = getpid();
  request->tid = pthread_self();
  request->dur = rand() % MAX_REQUEST_DUR + MIN_REQUEST_DUR;
  request->pl  = EMPTY_FIELD;

  IWANT_LOG(request);
}

static int
get_answer(int answer_fd, packet* answer, packet* request)
{
  if (read_sigsafe(answer_fd, answer, sizeof(packet)) <= 0) {
    // dead server
    FAILD_LOG(request);
    return 1;
  }
  request->pl = answer->pl;

  if (answer->pl == -1) {
    CLOSD_LOG(request);
    return 1;
  }

  IAMIN_LOG(request);
  return 0;
}

static void*
random_server_request(void* arg)
{
  packet request;
  gen_request(&request, (size_t)arg);

  char answer_fifo[80];
  sprintf(answer_fifo, "%s/%d.%ld", FIFO_DIR, request.pid, request.tid);
  // create private fifo
  if (fifo_mk_safe(answer_fifo, 0660)) {
    FAILD_LOG(&request);
    return NULL;
  }

  // make a request
  int write_ans;
  if ((write_ans = write_sigsafe(request_fd, &request, sizeof(packet))) < 0) {
    unlink(answer_fifo);
    FAILD_LOG(&request);
    return NULL;
  }
  else if (write_ans == 0) { // server closed
    is_open = 0;
    unlink(answer_fifo);
    CLOSD_LOG(&request);
    return NULL;
  }

  // open communication
  int answer_fd;
  if ((answer_fd = open(answer_fifo, O_RDONLY)) < 0) {
    unlink(answer_fifo);
    FAILD_LOG(&request);
    return NULL;
  }

  // get answer
  packet answer;
  if (get_answer(answer_fd, &answer, &request))
    is_open = 0;

  close(answer_fd);
  unlink(answer_fifo);
  return NULL;
}

static void
cleanup(void)
{
  free(threads);
  close(request_fd);
}

int
main(int argc, char* argv[])
{
  srand(time(NULL));
  init_u(argc, argv, &prog_props);
  /* open com to server */
  if ((request_fd = open(prog_props.fifoname, O_WRONLY | O_APPEND)) < 0) {
    fprintf(stderr, "Bathroom is closed.\n");
    exit(EXIT_FAILURE);
  }

  int max_threads = prog_props.nsecs * SEC2MILI / SEC_BETWEEN_REQUESTS;
  if ((threads = (pthread_t*)malloc(sizeof(pthread_t) * max_threads)) == NULL) {
    perror("Thread malloc failed.\n");
    exit(EXIT_FAILURE);
  }
  atexit(cleanup);
  setup_signal_handler();

  size_t i;
  for (i = 0; i < (size_t)max_threads && is_open; ++i) {
    if (pthread_create(&threads[i], NULL, random_server_request, (void*)i) !=
        0) {
      perror("Failed creating thread. Stopping..");
      break;
    }
    usleep(SEC_BETWEEN_REQUESTS * MICROS2MS);
  }
  close(request_fd);

  // join threads
  for (size_t j = 0; j < i; ++j)
    pthread_join(threads[j], NULL); // NO return expected

  exit(EXIT_SUCCESS);
}
