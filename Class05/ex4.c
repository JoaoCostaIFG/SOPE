#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ 0
#define WRITE 1

/* ls dir -laR | grep arg | sort */

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "USAGE: %s dir grep_arg\n", argv[0]);
    exit(1);
  }

  int fd[2], fd2[2], c;
  pipe(fd);
  pipe(fd2);

  // ls
  if (fork() > 0) {
    wait(NULL);
  } else {
    dup2(fd[WRITE], STDOUT_FILENO);
    execlp("ls", "ls", argv[1], "-laR", NULL);
    exit(1);
  }

  // grep
  close(fd[WRITE]);
  if (fork() > 0) {
    wait(NULL);
  } else {
    dup2(fd[READ], STDIN_FILENO);
    dup2(fd2[WRITE], STDOUT_FILENO);
    execlp("grep", "grep", argv[2], NULL);
    exit(1);
  }

  // sort
  close(fd2[WRITE]);
  if (fork() > 0) {
    wait(NULL);
  } else {
    dup2(fd2[READ], STDIN_FILENO);
    execlp("sort", "sort");
    exit(1);
  }

  return 0;
}
