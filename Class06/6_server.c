#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "6.h"

#define MAX_THREADS 10

static thread_ans threads[MAX_THREADS];

void *calc(void *thread) {
  thread_ans *ans = (thread_ans *)thread;

  ans->ans.sum = ans->req.a + ans->req.b;
  ans->ans.sub = ans->req.a - ans->req.b;
  ans->ans.mul = ans->req.a * ans->req.b;
  ans->ans.div = ((float)ans->req.a) / ans->req.b;

  write(ans->fd, &ans->ans, sizeof(answer));
  close(ans->fd);

  ans->fd = -1;
  return NULL;
}

void cleanup(void) {
  puts("\nClosing server");

  unlink(FIFO_REP);
  for (int i = 0; i < MAX_THREADS; ++i) {
    if (threads[i].fd < 0)
      continue;
    pthread_join(threads[i].thread, NULL);
  }

  puts("Closed server");
  exit(0);
}

void sigint_handler(int signo) { cleanup(); }

int main(void) {
  /* init */
  for (int i = 0; i < MAX_THREADS; ++i) {
    threads[i].fd = -1;
  }
  mkfifo(FIFO_REP, 0660);
  signal(SIGINT, sigint_handler);

  int rfd = open(FIFO_REP, O_RDONLY);
  if (rfd == -1) {
    perror("failed opening server fifo");
    return 1;
  }

  int i = 0, read_ret;
  /*
   * FILE* fff = fopen(FIFO_REP, "rb");
   * if (fff == NULL) {
   *   puts("I failed");
   *   exit(1);
   * }
   */
  while (1) {
    /* fread(&threads[i].req, sizeof(request), 1, fff); */
    while ((read_ret = read(rfd, &threads[i].req, sizeof(request))) == 0) {
      continue;
    }
    if (read_ret < 0) {
      perror("server fifo reading failure. Exiting..\n");
      break;
    }

    // got request
    printf("Got request: %d %d %s\n", threads[i].req.a, threads[i].req.b,
           threads[i].req.fifo);
    if (threads[i].req.a == 0 && threads[i].req.b == 0) { // 0, 0 => die
      puts("Got kill cmd. Exiting..\n");
      break;
    }

    if ((threads[i].fd = open(threads[i].req.fifo, O_WRONLY)) < 0) {
      perror("failed opening client fifo");
    } else {
      pthread_create(&threads[i].thread, NULL, calc, (void *)&threads[i]);
      ++i;
      if (i == MAX_THREADS)
        i = 0;
    }
  }

  close(rfd);
  cleanup();
  return 0;
}
