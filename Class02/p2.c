#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define USAGE_MSG "copy source destination\n"
#define SRC_F_MSG "failed opening source file\n"
#define DEST_F_MSG "failed destination source file\n"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    write(STDIN_FILENO, USAGE_MSG, strlen(USAGE_MSG));
    exit(1);
  }

  int src_fd, dest_fd;
  if ((src_fd = open(argv[1], O_RDONLY)) == -1) {
    write(STDIN_FILENO, SRC_F_MSG, strlen(SRC_F_MSG));
    exit(1);
  }
  if ((dest_fd = open(argv[2], O_WRONLY | O_APPEND | O_CREAT | O_EXCL, 0644)) == -1) {
    write(STDIN_FILENO, DEST_F_MSG, strlen(DEST_F_MSG));
    exit(1);
  }

  char ch;
  while (read(src_fd, &ch, 1)) {
    write(dest_fd, &ch, 1);
  }

  close(src_fd);
  close(dest_fd);
  return 0;
}
