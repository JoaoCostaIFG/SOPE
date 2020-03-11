#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  DIR *dirp;
  struct dirent *direntp;
  struct stat stat_buf;
  char *str;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s dir_name\n", argv[0]);
    exit(1);
  }

  if ((dirp = opendir(argv[1])) == NULL) {
    perror(argv[1]);
    exit(2);
  }
  chdir(argv[1]);

  while ((direntp = readdir(dirp)) != NULL) {
    if (stat(direntp->d_name, &stat_buf) == -1) {
      perror("stroke no stat");
      exit(1);
    }
    if (S_ISREG(stat_buf.st_mode))
      str = "regular";
    else if (S_ISDIR(stat_buf.st_mode))
      str = "directory";
    else
      str = "other";
    printf("%-10lu %-5ld %-25s - %s\n", direntp->d_ino, stat_buf.st_size,
           direntp->d_name, str);
  }

  closedir(dirp);
  return 0;
}
