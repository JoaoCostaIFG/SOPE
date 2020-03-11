#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define PROMPT "jfjsh> "
#define PROMPT_LEN 7
#define MAX_CMD_LEN 1000
#define MAX_ARGS 36
#define MAX_TOKEN_SIZE 32

#define QUIT_CMD "quit"
#define QUIT_CMD_LEN 4
#define FAIL_CMD "input cmd failed\n"
#define FAIL_CMD_LEN 17
#define FAIL_FILE_CMD "file creation/writting failed: "
#define FAIL_FILE_CMD_LEN 32
#define FILE_OUT_CMD ">"
#define FILE_APPEND_CMD ">>"

#define GREETING "Type any command (quit for exit)\n"
#define GREETING_LEN 34

int main(void) {
  pid_t pid;
  int st, read_res = 1;
  int i, j, k;
  char input_cmd[MAX_CMD_LEN + 1];
  char *command[MAX_ARGS + 1];
  int fd = -1;

  /* greetings messages */
  write(STDOUT_FILENO, GREETING, GREETING_LEN);

  /* command input loop */
  while (read_res) {
    /* check and delimit read string */
    write(STDOUT_FILENO, PROMPT, PROMPT_LEN);
    if ((read_res = read(STDIN_FILENO, input_cmd, MAX_CMD_LEN)) == -1 ||
        read_res == 0 || input_cmd[0] == '\n')
      continue;
    input_cmd[--read_res] = '\0';     // strip '\n'
    if (!strcmp(input_cmd, QUIT_CMD)) // quit input_cmd
      break;

    /* divide in tokens */
    for (i = 0, j = 0;i < MAX_ARGS && j < read_res; ++i) {
      command[i] = (char *)malloc(sizeof(char) * (MAX_TOKEN_SIZE + 1));
      if (!command[i]) {
        write(STDOUT_FILENO, FAIL_CMD, FAIL_CMD_LEN);
        exit(1);
      }

      for (k = 0; input_cmd[j] != ' ' && k < 32; ++j, ++k) {
        if (input_cmd[j] == '\0') {
          command[i + 1] = NULL;
          break;
        }
        command[i][k] = input_cmd[j];
      }
      command[i][k] = 0;
      ++j;
    }

    /* check if output should be redirected */
    if (i >= 2) {
      int flags = 0;
      if (!strcmp(command[i - 2], FILE_APPEND_CMD)) // APPEND
        flags = O_WRONLY | O_CREAT | O_APPEND;
      else if (!strcmp(command[i - 2], FILE_OUT_CMD)) // TRUNC
        flags = O_WRONLY | O_CREAT | O_APPEND | O_TRUNC;

      if (flags) {
        if ((fd = open(command[i - 1], flags, 0644)) == -1) {
          write(STDOUT_FILENO, FAIL_FILE_CMD, FAIL_FILE_CMD_LEN);
          write(STDOUT_FILENO, command[i - 1], strlen(command[i - 1]));
          continue;
        }
        free(command[i - 2]);
        free(command[i - 1]);
        command[i - 2] = NULL;
        i -= 2;
      }
    }

    /* call process */
    pid = fork();
    switch (pid) {
    case -1:
      // failed fork
      write(STDOUT_FILENO, FAIL_CMD, FAIL_CMD_LEN);
      break;
    case 0:
      // child
      if (fd != -1) // redirect output
        dup2(fd, STDOUT_FILENO);
      execvp(command[0], command);
      exit(1);
      break;
    default:
      // parent
      wait(&st);
      if (fd != -1)
        close(fd);
      for (int n = 0; n < i; ++n)
        free(command[n]);
      if (WIFEXITED(st) && WEXITSTATUS(st) != 0)
        write(STDOUT_FILENO, FAIL_CMD, FAIL_CMD_LEN);
    }
  }

  return 0;
}
