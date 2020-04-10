#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define stupidenv "stupidname"
#define BIGFD 999

void print_fds(char* name) {
  fprintf(stderr, "%s PID: %d\n", name, getpid());

  char proc_path[50];
  sprintf(proc_path, "/proc/%d/fd/", getpid());
  char path[100];

  char fd_info[PIPE_BUF + 1];
  int fd_info_len;
  struct dirent *direntp;
  DIR *dirp = opendir(proc_path);

  while ((direntp = readdir(dirp))) {
    if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, ".."))
      continue;

    strcpy(path, proc_path);
    strcat(path, direntp->d_name);

    if ((fd_info_len = readlink(path, fd_info, PIPE_BUF)) == -1) {
      fprintf(stderr, "readlink failed (broken symlink)\n");
      continue;
    }
    fd_info[fd_info_len] = '\0';

    fprintf(stderr, "%s - %s - %s\n", name, direntp->d_name, fd_info);
  }
}

int main(int argc, char **argv) {
  if (getenv(stupidenv) != NULL) { // exec'ed
    print_fds("execed kid");
    return 0;
  } else
    setenv(stupidenv, stupidenv, 1);

  int fd[2];
  pipe(fd);
  if (!fork()) { // child
    dup2(fd[1], BIGFD);
    print_fds("kid");

    execv(argv[0], argv);
  } else { // parent
    print_fds("parent");
    wait(NULL);
  }

  return 0;
}
