#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  if (argc < 3) {
    puts("kys");
    return 1;
  }

  srand(time(NULL));
  int n1 = atoi(argv[1]);
  int n2 = atoi(argv[2]);
  if (n1 < n2) {
    puts("kys2");
    return 1;
  }

  int randini = rand() % n1;
  while (randini != n2) {
    randini = rand() % n1;
    printf("%d\n", randini);
  }

  
  struct tms buf;
  times(&buf);
  int tick_sec = sysconf(_SC_CLK_TCK);
  printf("tempo cpu process: %ld\n", tick_sec * buf.tms_utime);
  printf("tempo cpu system: %ld\n", tick_sec * buf.tms_stime);


  return 0;
}
