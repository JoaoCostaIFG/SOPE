#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIFO_REP "/tmp/fifo_req"
#define FIFO_ANS "/tmp/fifo_ans"


void read_ans(int rfd) {
  static int ops[] = { '+', '-', '*', '/' };
  static int curr_op = 0;

  int ans_i;
  float ans_f;

  char type;
  read(rfd, &type, 1);
  switch (type) {
  case 'i':
    read(rfd, &ans_i, sizeof(int));
    printf("%c: %d\n", ops[curr_op], ans_i);
    break;
  case 'f':
    read(rfd, &ans_f, sizeof(float));
    printf("%c: %f\n", ops[curr_op], ans_f);
    break;
  case 'n':
    printf("Invalid operation for given operands");
    break;
  default:
    printf("Unknown answer\n");
    break;
  }

  ++curr_op;
}

int main(void) {
  int wfd = open(FIFO_REP, O_WRONLY);
  int rfd = open(FIFO_ANS, O_RDONLY);
  if (rfd == -1 || wfd == -1) {
    perror("failed opening fifos");
    return 1;
  }

  int a, b;
  printf("num a and b? [0 0 to kill] ");
  scanf("%d", &a);
  scanf("%d", &b);
  printf("nums: %d %d\n", a, b);
  write(wfd, &a, sizeof(int));
  write(wfd, &b, sizeof(int));

  if (a == 0 && b == 0) {
    puts("killed server\n");
    return 0;
  }

  for (int i = 1; i <= 4; ++i) {
    read_ans(rfd);
  }

  close(rfd);
  close(wfd);
  return 0;
}
