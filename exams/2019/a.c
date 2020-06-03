#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

sem_t sem, sem2;
int goOn = 1;

void *p1(void *a) {
  while (goOn) {
    sem_wait(&sem);
    write(STDOUT_FILENO, "1 ", 2);
    sem_post(&sem2);
  }

  return NULL;
}

void *p2(void *a) {
  while (goOn) {
    sem_wait(&sem2);
    write(STDOUT_FILENO, "2 ", 2);
    sem_post(&sem);
  }

  return NULL;
}

int main(void) {
  sem_init(&sem, 0, 1);
  sem_init(&sem2, 0, 0);

  pthread_t t1, t2;
  pthread_create(&t1, NULL, p1, NULL);
  pthread_create(&t2, NULL, p2, NULL);

  sleep(1);
  goOn = 0;

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  return 0;
}
