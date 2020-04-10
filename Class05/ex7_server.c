#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <tgmath.h>
#include <unistd.h>

#define FIFO_REP "/tmp/fifo_req"
#define FIFO_ANS "/tmp/fifo_ans"

int main(void) {
  mkfifo(FIFO_REP, 0660);
  mkfifo(FIFO_ANS, 0660);
  int rfd = open(FIFO_REP, O_RDONLY);
  int wfd = open(FIFO_ANS, O_WRONLY);

  int a, b;
  int ans_i;
  float ans_f;
  do {
    while (1) {
      if (read(rfd, &a, sizeof(int)) == 0)
        continue;
      if (read(rfd, &b, sizeof(int)) == 0)
        continue;
      break;
    }
    printf("Got: %d %d\n", a, b);
    if (a == 0 && b == 0) {
      puts("Got kill cmd. Exiting..\n");
      break;
    }

    // summ
    ans_i = a + b;
    write(wfd, "i", 1);
    write(wfd, &ans_i, sizeof(int));

    // diff
    ans_i = a - b;
    write(wfd, "i", 1);
    write(wfd, &ans_i, sizeof(int));

    // prod
    ans_i = a * b;
    write(wfd, "i", 1);
    write(wfd, &ans_i, sizeof(int));

    // quoc
    if (b == 0) {
      write(wfd, "n", 1);
    } else {
      ans_f = (float)a / b;
      if (ans_f == ceil(ans_f)) { // is int
        ans_i = (int)ans_f;
        write(wfd, "i", 1);
        write(wfd, &ans_i, sizeof(int));
      } else {
        write(wfd, "f", 1);
        write(wfd, &ans_f, sizeof(float));
      }
    }
    puts("End cycle");
  } while (1);

  unlink(FIFO_REP);
  unlink(FIFO_ANS);
  close(rfd);
  close(wfd);
  return 0;
}
