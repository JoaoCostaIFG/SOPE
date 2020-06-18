#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "a.h"

int main(int argc, char *argv[]) {
  // criar shmem no path dado
  int fd = shm_open(shmpath, O_CREAT | O_RDWR | O_EXCL, 0660);
  if (fd < 0) {
    fprintf(stderr, "PROC1: died on shm open\n");
    exit(1);
  }

  // definir size da shared mem == size da struct acima
  if (ftruncate(fd, sizeof(shmbuf)) == -1) {
    fprintf(stderr, "PROC1: died on ftruncate\n");
    exit(1);
  }

  // mapear a shmem no nosso address space (neste caso toda pk escolhemos o size
  // da struct). Reparem que mmap retorna void* e eu fiz o cast implicito no
  // tipo da var
  shmbuf *pa =
      mmap(NULL, sizeof(shmbuf), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (pa == MAP_FAILED) {
    fprintf(stderr, "PROC1: morreu no mmap\n");
    exit(1);
  }

  // initializar data da str
  strcpy(pa->buf, "123456789");
  // 2 semaforos definidos como inter-process (1), initializados a 0
  if (sem_init(&pa->sem1, 1, 0) ==  -1) {
    fprintf(stderr, "PROC1: Morri a dar init ao sem 1\n");
    exit(1);
  }
  if (sem_init(&pa->sem2, 1, 0) == -1) {
    fprintf(stderr, "PROC1: Morri a dar init ao sem 2\n");
    exit(1);
  }

  // esperamos pelo segundo process
  printf("PROC1: Ready and waiting for process 2\n");
  sem_wait(&pa->sem1);
  // mudamos a data no buffer partilhado
  printf("PROC1: Wrote nem string and warned process 2\n");
  strcpy(pa->buf, "abcdefghi");
  // avisamos que estamos prontos
  sem_post(&pa->sem2);

  // podemos dar unlink a vontade pk o buffer so e efetivamente destruido quando
  // toda a gente o "largar"
  shm_unlink(shmpath);

  return 0;
}
