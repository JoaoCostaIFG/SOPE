#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "a.h"

int main(int argc, char *argv[]) {
  // abrir shmem no path dado
  int fd = shm_open(shmpath, O_RDWR, 0);
  if (fd < 0) {
    fprintf(stderr, "PROC2: died on shm open\n");
    exit(1);
  }

  // mapear a shmem no nosso address space (neste caso toda pk escolhemos o size
  // da struct). Reparem que mmap retorna void* e eu fiz o cast implicito no
  // tipo da var
  // WARNING: os PROT_* e assim têm de ser iguais entre todos os procs se nao a
  // data pode vir fodida
  shmbuf *pa =
      mmap(NULL, sizeof(shmbuf), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (pa == MAP_FAILED) {
    fprintf(stderr, "PROC2: morreu no mmap\n");
    exit(1);
  }

  printf("PROC2: O buffer contem a string: %s\n", pa->buf);
  // vamos avisar o primeiro processo que ja pode mudar a data e esperar que ele
  // o faca
  sem_post(&pa->sem1);
  printf("PROC2: Avisei o proc 1 e agora vou esperar\n");
  fflush(stdout);
  sem_wait(&pa->sem2);

  printf("PROC2: Agora o buffer contem a string: %s\n", pa->buf);

  // podemos dar unlink a vontade pk o buffer so e efetivamente destruido quando
  // toda a gente o "largar"
  shm_unlink(shmpath);

  return 0;
}
