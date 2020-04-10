#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  mkfifo("carlos", 0640);
  int res;

  if (!fork()) {
    res = 0;
    int fd_r = open("carlos", O_RDONLY, 0400);

    read(fd_r, &res, sizeof(int));
    printf("SOU FILHO: %d\n", res);

    if (read(fd_r, &res, sizeof(int)) == 0)
      printf("FILHO EOF\n");
    close(fd_r);
    if (read(fd_r, &res, sizeof(int)) == -1)
      printf("FILHO READ ERR\n");
  } else {
    res = 123456789;
    int fd_w = open("carlos", O_WRONLY, 0200);

    write(fd_w, &res, sizeof(int));
    close(fd_w);
    printf("SOU PAI: %d\n", res);

    unlink("carlos");
  }

  return 0;
}
