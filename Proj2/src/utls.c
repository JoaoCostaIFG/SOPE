#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "include/utls.h"

int
read_sigsafe(const int fd, void* buf, const size_t size)
{
  int ret;
  do {
    errno = 0;
    ret   = read(fd, buf, size);
  } while (errno == EINTR);

  return ret;
}

int
write_sigsafe(const int fd, const void* const buf, const size_t size)
{
  int ret;
  do {
    errno = 0;
    ret   = write(fd, buf, size);
  } while (errno == EINTR);

  return ret;
}

int
fifo_mk_safe(const char* const name, __mode_t mode)
{
  errno = 0;
  if (mkfifo(name, mode)) {
    if (errno != EEXIST) {
      perror(name);
      return 1;
    }
    else if (unlink(name) || mkfifo(name, mode)) {
      /* FIFO already exists => trying to delete and create it again */
      perror(name);
      return 1;
    }
  }

  return 0;
}

int
fifo_mk_open(const char* const name, __mode_t mode, const int o_flags)
{
  if (fifo_mk_safe(name, mode))
    return -1;

  int fd;
  if ((fd = open(name, O_RDONLY)) < 0) {
    perror(name);
    if (unlink(name))
      perror(name);
  }

  return fd;
}

