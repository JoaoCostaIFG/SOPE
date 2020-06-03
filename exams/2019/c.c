// NOTE: for simplicity, error treatment was ommited
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int numThreads;
int turn = 0; // The first thread to run must have thrIndex=turn=0

void *thr(void *arg) {
  int thrIndex = *(int *)arg; // The effective indexes are 0,1,2,...

  while (1) {
    pthread_mutex_lock(&mutex);

    while (thrIndex != turn) {
      pthread_cond_wait(&cond, &mutex);
    }
    printf("%d ", thrIndex + 1); // The numbers shown are 1,2,3,...
    fflush(stdout);
    turn = (turn + 1) % numThreads;

    for (int i = 0; i < numThreads; ++i)
      pthread_cond_signal(&cond);

    pthread_mutex_unlock(&mutex);
  }

  return NULL;
}

int main() {
  printf("Number of threads ? ");
  scanf("%d", &numThreads);
  int *arg = (int *)malloc(sizeof(int) * numThreads);
  pthread_t *tid = (pthread_t *)malloc(sizeof(pthread_t) * numThreads);
  for (int i = 0; i < numThreads; i++) {
    arg[i] = i;
    pthread_create(&tid[i], NULL, thr, (void *)&arg[i]);
  }
  pthread_exit(NULL);
}
