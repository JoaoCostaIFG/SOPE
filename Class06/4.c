#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define N 50

void *pthread_fun(void *arg) {
  sleep(1);
  fprintf(stderr, "I'm thread: %lu\n", pthread_self());
  return arg;
}

int main(void) {
  pthread_t threads[N];
  for (int i = 0; i < N; ++i)
    pthread_create(&threads[i], NULL, pthread_fun, (void *)i);

  int *ret;
  for (int i = 0; i < N; ++i) {
    pthread_join(threads[i], (void **)&ret);
    printf("Got thread: %d\n", (int)ret);
  }

  return 0;
}
