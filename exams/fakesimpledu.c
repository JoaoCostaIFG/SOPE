#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc == 1) {
    puts("kys");
    exit(1);
  }

  int n = 0;
  char *dirr = argv[1];
  int lsfork = fork();
  switch (lsfork) {
  case -1:
    puts("olha morri no 1st fork");
    exit(1);
    break;
  case 0:
    execlp("ls", "ls", "-lasR", dirr);
    exit(1);
    break;
  default:
    ++n;
    break;
  }

  DIR *dirp = opendir(dirr);
  struct dirent *dirent;
  struct stat statbuf;
  while ((dirent = readdir(dirp))) {
    if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, "..")) {
      dirent = readdir(dirp);
      continue;
    }

    char argv1[256];
    strcpy(argv1, dirr);
    strcat(argv1, "/");
    strcat(argv1, dirent->d_name);

    stat(argv1, &statbuf);
    if (S_ISDIR(statbuf.st_mode)) {
      int pid = fork();
      switch (pid) {
      case -1:
        puts("olha morri no fork");
        exit(1);
        break;
      case 0:
        execl(argv[0], argv[0], argv1, NULL);
        break;
      default:
        ++n;
        break;
      }
    }
  }

  for (int i = 0; i <= n; ++i)
    wait(NULL);

  return 0;
}
