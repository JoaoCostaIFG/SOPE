#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define USAGE_MSG "copy file_path\n"
#define SRC_F_MSG "failed opening source file\n"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    write(STDIN_FILENO, USAGE_MSG, strlen(USAGE_MSG));
    exit(1);
  }

  int src_fd;
  if ((src_fd = open(argv[1], O_RDONLY)) == -1) {
    write(STDIN_FILENO, SRC_F_MSG, strlen(SRC_F_MSG));
    exit(1);
  }

  char ch;
  while (read(src_fd, &ch, 1)) {
    write(STDOUT_FILENO, &ch, 1);
  }

  close(src_fd);
  return 0;
}
