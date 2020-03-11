#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void exitarino1(void) {
  puts("exit 1 da amizade!");
}

void exitarino2(void) {
  puts("exit 2 da amizade!");
  exit(0);
}

int main(int argc, char *argv[], char *envp[])
{
  atexit(exitarino1);
  atexit(exitarino2);
  puts("sds");
  return 0;
}
