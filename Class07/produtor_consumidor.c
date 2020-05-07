#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define SHARED 0 /* sem. is shared between threads */

void *producer(void *); /* the two threads */
void *consumer(void *);

sem_t empty, full; /* the global semaphores */
int data;          /* shared buffer */
int numIters;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <numIters>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  pthread_t pid, cid;
  numIters = atoi(argv[1]);
  sem_init(&empty, SHARED, 1); /* sem empty = 1 */
  sem_init(&full, SHARED, 0);  /* sem full = 0 */

  printf("Main started.\n");
  pthread_create(&pid, NULL, producer, NULL);
  pthread_create(&cid, NULL, consumer, NULL);
  pthread_join(pid, NULL);
  pthread_join(cid, NULL);

  printf("Main done.\n");
  return 0;
}

/* Put items (1, ..., numIters) into the data buffer and sum them */
void *producer(void *arg) {
  int total = 0, produced;

  printf("Producer running\n");
  for (produced = 1; produced <= numIters; produced++) {
    sem_wait(&empty);
    data = produced;
    total = total + data;
    sem_post(&full);
  }

  printf("Producer: total produced is %d\n", total);
  return NULL;
}

/* Get values from the data buffer and sum them */
void *consumer(void *arg) {
  int total = 0, consumed;

  printf("Consumer running\n");
  for (consumed = 1; consumed <= numIters; consumed++) {
    sem_wait(&full);
    total = total + data;
    sem_post(&empty);
  }

  printf("Consumer: total consumed is %d\n", total);
  return NULL;
}
