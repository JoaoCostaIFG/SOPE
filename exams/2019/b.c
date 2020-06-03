#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
  if (argc == 1) {
    puts("fu file name");
    exit(1);
  }

  char *out = argv[1];
  int f = open(out, O_CREAT | O_APPEND | O_WRONLY, 0644);

  int fd[2];
  pipe(fd);
  dup2(f, STDOUT_FILENO);
  dup2(f, STDERR_FILENO);

  int pid = fork();
  switch (pid) {
  case 0:
    close(fd[1]);
    dup2(fd[0], STDIN_FILENO);
    execlp("bc", "bc", "-qi", NULL);
    fprintf(stderr, "exec morreu nigga\n");
    exit(1);
    break;
  case -1:
    fprintf(stderr, "fork morreu nigga\n");
    exit(1);
    break;
  default:
    close(fd[0]);
    break;
  }

  char str[256];
  int goOn = 1;
  int n;
  while (goOn) {
    n = read(STDIN_FILENO, str, 255);
    str[n] = '\0';
    if (n == 1 && str[0] == '\n') {
      goOn = 0;
      break;
    }

    write(STDOUT_FILENO, str, n - 1);
    write(STDOUT_FILENO, " = ", 3);
    fflush(stdout);

    write(fd[1], str, n);
    fflush(stdout);
  }

  close(fd[1]);
  wait(NULL);

  return 0;
}
