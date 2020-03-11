#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  int st;
  int pid = fork();
  if (pid < 0) {
    perror("fork");
  } else if (pid == 0) { // is_child
    pid = fork();
    if (pid < 0) {
      perror("fork");
    } else if (pid == 0) { // is_child
      write(STDOUT_FILENO, "Hello ", 6);
    } else { // is_parent
      wait(&st);
      write(STDOUT_FILENO, "my ", 3);
    }
  } else { // is_parent
    wait(&st);
    write(STDOUT_FILENO, "friends!\n", 9);
  }

  return 0;
}
