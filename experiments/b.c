#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void sig_alrm(int);
static jmp_buf env_alrm;

#define MAXLINE 1024

int main(void) {
  int n;
  char line[MAXLINE];
  if (signal(SIGALRM, sig_alrm) == SIG_ERR)
    fprintf(stderr, "signal(SIGALRM) error\n");
  if (setjmp(env_alrm) != 0)
    fprintf(stderr, "read timeout\n");

  printf("alarm - %d\n", alarm(1));
  if ((n = read(STDIN_FILENO, line, MAXLINE)) < 0)
    fprintf(stderr, "read error");
  alarm(0);

  write(STDOUT_FILENO, line, n);
  exit(0);
}

static void sig_alrm(int signo) { longjmp(env_alrm, 1); }
