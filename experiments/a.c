#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[], char *env[]) {
  for (int i = 0; env[i] != NULL; ++i)
    printf("%s\n", env[i]);
  setenv("abcdefghijklmnopqrstuvwxyz", "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 1);
  for (int i = 0; env[i] != NULL; ++i)
    printf("%s\n", env[i]);
  exit(1);



  if (getenv("ABCDEFGHIJKL") == NULL) {
    setenv("ABCDEFGHIJKL", "abc", 1);
  } else {
    fprintf(stdout, "Sou out exec %d %d.\n", getpid(), getppid());
    fprintf(stderr, "Sou err exec %d %d.\n", getpid(), getppid());
    exit(0);
  }

  int fd[2];
  pipe(fd);

  pid_t pid = fork();
  if (pid == 0) { // child
    fprintf(stdout, "Sou out filho %d %d.\n", getpid(), getppid());
    fprintf(stderr, "Sou err filho %d %d.\n", getpid(), getppid());

    dup2(fd[1], STDOUT_FILENO);

    execv(argv[0], argv);
  } else if (pid > 0) { // pai
    fprintf(stdout, "Sou out pai %d.\n", getpid());
    fprintf(stderr, "Sou err pai %d.\n", getpid());
  } else {
    fprintf(stderr, "Fork fail.\n");
  }

  wait(NULL);
  return 0;
}
