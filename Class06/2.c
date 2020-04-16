// PROGRAMA p02.c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N 50000

int write_lefts = N;
pthread_mutex_t writes_left_mutex = PTHREAD_MUTEX_INITIALIZER;

void *thrfunc(void *arg) {
  int *num_writes = (int *)malloc(sizeof(int));
  *num_writes = 0;

  fprintf(stderr, "Starting thread %s\n", (char *)arg);
  while (write_lefts > 0) {
    pthread_mutex_lock(&writes_left_mutex);
    --write_lefts;
    pthread_mutex_unlock(&writes_left_mutex);

    (*num_writes) += 1;
    write(STDERR_FILENO, arg, 1);
  }
  return num_writes;
}

int main() {
  pthread_t ta, tb;

  char a = '1', b = '2';
  pthread_create(&ta, NULL, thrfunc, &a);
  pthread_create(&tb, NULL, thrfunc, &b);

  int *reta, *retb;
  pthread_join(ta, (void **)&reta);
  pthread_join(tb, (void **)&retb);

  int total = *reta + *retb;
  printf("\nThread 1 wrote: %d\nThread 2 wrote: %d\nTotal: %d\nDiff: %d\n",
         *reta, *retb, total, N - total);

  free(reta);
  free(retb);

  return 0;
}
