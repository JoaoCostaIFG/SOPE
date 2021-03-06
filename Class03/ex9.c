#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[], char *envp[]) {
  pid_t pid;
  if (argc != 2) {
    printf("usage: %s dirname\n", argv[0]);
    exit(1);
  }
  pid = fork();
  if (pid > 0) {
    FILE* fp = fopen("aa", "w+");
    fprintf(fp ,"%d\n", pid);
    fclose(fp);
    printf("My child is going to execute the command \"ls -laR %s\"\n", argv[1]);
    int st;
    wait(&st);
    if (WIFSTOPPED(st))
      puts("is stoped");
    if (WIFEXITED(st))
      printf("Exited with status: %d\n", WEXITSTATUS(st));
    if (WIFSIGNALED(st))
      printf("Signaled with status: %d\n", WTERMSIG(st));
  } else if (pid == 0) {
    /* execlp("ls", "ls", "-laR", argv[1]); */
    char *command[] = {"/usr/bin/ls", "-laR", argv[1], 0};
    execve("/usr/bin/ls", command, envp);
    printf("Command not executed !\n");
    exit(1);
  }
  exit(0);
}
