#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// compile a program using gcc
int main(int argc, char *argv[]) {
  char prog[20];
  sprintf(prog, "%s.c", argv[1]);
  execlp("gcc", "gcc", prog, "-Wall", "-o", argv[1], NULL);

  // only reaches this point in case of failure
  printf("Compilation failed\n");
  exit(1);
}
