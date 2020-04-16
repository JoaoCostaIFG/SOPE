#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "6.h"

void get_ans(int rfd) {
  answer ans;
  read(rfd, &ans, sizeof(answer));
  printf("+: %d\n-: %d\n*: %d\n/: %.2f\n", ans.sum, ans.sub, ans.mul, ans.div);
}

int main(void) {
  int wfd = open(FIFO_REP, O_WRONLY);
  if (wfd == -1) {
    perror("failed opening server fifo");
    return 1;
  }

  request req;
  printf("num a and b? [0 0 to kill] ");
  scanf("%d", &req.a);
  scanf("%d", &req.b);
  printf("nums: %d %d\n", req.a, req.b);

  // kill server
  if (req.a == 0 && req.b == 0) {
    write(wfd, &req, sizeof(request));
    puts("killed server\n");
    return 0;
  }

  sprintf(req.fifo, "%s%d", FIFO_ANS, getpid());
  mkfifo(req.fifo, 0660);

  write(wfd, &req, sizeof(request));
  close(wfd);

  int rfd = open(req.fifo, O_RDONLY);
  get_ans(rfd);

  close(rfd);
  unlink(req.fifo);
  return 0;
}
