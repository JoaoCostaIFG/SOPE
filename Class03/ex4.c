#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
  int st;
  int pid = fork();

  switch (pid) {
  case -1:
    perror("fork");
    break;
  case 0:
    write(STDOUT_FILENO, "Hello ", 6);
    break;
  default:
    wait(&st);
    write(STDOUT_FILENO, "world!\n", 7);
    break;
  }

  return 0;
}
