#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[], char *envp[]) {
  pid_t pid;
  if (argc != 2 && argc != 3) {
    printf("usage: %s dirname\n", argv[0]);
    exit(1);
  }
  pid = fork();
  if (pid > 0) {
    printf("My child is going to execute the command \"ls -laR %s\"\n", argv[1]);
    int st;
    wait(&st);
  } else if (pid == 0) {
    if (argv[2]) {
      int fd = open(argv[2], O_EXCL | O_WRONLY | O_CREAT | O_APPEND, 0644);
      if (fd < 0) {
        perror("file open/creation");
        exit(1);
      }
      dup2(fd, STDOUT_FILENO);
    }
    char *command[] = {"/usr/bin/ls", "-laR", argv[1], 0};
    execve("/usr/bin/ls", command, envp);
    printf("Command not executed !\n");
    exit(1);
  }
  exit(0);
}
