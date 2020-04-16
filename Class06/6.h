#ifndef H_6_H
#define H_6_H

#include <sys/types.h>

#define FIFO_REP "/tmp/fifo_req"
#define FIFO_ANS "/tmp/fifo_ans_"

typedef struct {
  int a;
  int b;
  char fifo[100];
} request;

typedef struct {
  int sum, sub, mul;
  float div;
} answer;

typedef struct {
  request req;
  int fd;
  answer ans;
  pthread_t thread;
} thread_ans;

enum OPS {
  SUM = 0,
  SUB = 1,
  MUL = 2,
  DIV = 3,
};

#endif // H_6_H
