/* NOT FINISHED */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ 0
#define WRITE 1
#define PIPE_BUFF 4096

/* ls dir -laR | grep arg | sort */

int main(int argc, char *argv[]) {
  int fd[2];
  pipe(fd);
  int c, j = 0;
  char out[PIPE_BUFF];
  char **args;

  for (int i = 0; i < argc; ++i) {
    if (!strcmp(argv[i], "|")) {
      args[j] = NULL;
      j = 0;
    } else {
      args[j] = (char*) malloc(sizeof(char) * (strlen(args[i]) + 1));
      strcpy(args[j++], argv[i]);
    }

    switch (fork()) {
    case 0:
      close(fd[READ]);
      dup2(fd[WRITE], STDOUT_FILENO);

      execvp(args[0], args);
      exit(1);
      break;
    default:
      wait(NULL);
      break;
    }
  }

  return 0;
}
