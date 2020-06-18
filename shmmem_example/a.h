#ifndef A_H
#define A_H

#include <semaphore.h>

//  void* pa = mmap(NULL, 256, PROT_READ | PROT_WRITE, MAP_SHARED |
//  MAP_ANONYMOUS, -1, 0);

// nome deve comecar por '/' e ter menos de 255 chars (o char 256 fico '\0')
// se houver algum problema e a shmem n for destruida, facam o comando:
// unlink /dev/shm/shmabc
// que e o sitio onde ela deve estar
#define shmpath "/shmabc"
#define BUF_SIZE 10

struct shmbuf {
  sem_t sem1;         /* POSIX unnamed semaphore */
  sem_t sem2;         /* POSIX unnamed semaphore */
  char buf[BUF_SIZE]; /* Data being transferred */
};
typedef struct shmbuf shmbuf;

#endif // A_H
