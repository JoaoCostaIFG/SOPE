#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "include/logs.h"
#include "include/sigs.h"

static pid_t pg_id; /**< process group of children */

void sigint_handler(int signum) {
  if (signum != SIGINT)
    return;
  write_recvsig_log(SIGINT);

  // pause children
  kill(-pg_id, SIGSTOP);
  write_sendsig_log(SIGSTOP, -pg_id);
  // get user choice
  puts("");
  int ans;
  do {
    // loop until either 'n', 'N', 'y' or 'Y' is read from stdin
    printf("Do you really wish to stop execution? [y/n] ");
  } while ((ans = getchar()) != EOF &&
           (tolower(ans) != 'y' && tolower(ans) != 'n'));
  getchar();

  // process decision
  if (tolower(ans) == 'n') { // unpause children
    puts("Continue..");
    kill(-pg_id, SIGCONT);
    write_sendsig_log(SIGCONT, -pg_id);
  } else { // kill children
    puts("Exiting..");
    kill(-pg_id, SIGTERM);
    write_sendsig_log(SIGTERM, -pg_id);
    exit_log(EXIT_SUCCESS);
  }
}

void set_child_sig(void) {
  if (getpgrp() == get_pg_id()) // permission already set
    return;

  setpgid(0, pg_id); // set process group id
  /* ignore SIGINT */
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_NOCLDSTOP;
  if (sigaction(SIGINT, &sa, NULL) == -1)
    exit_perror_log(SIG_FAIL, "ignoring SIGINT handler failed.");
}

void set_grandparent_sig(void) {
  set_pg_id(getpid() + 1); // save process group id for children
  /* handle SIGINT */
  struct sigaction sa;
  sa.sa_handler = &sigint_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_NOCLDSTOP;
  if (sigaction(SIGINT, &sa, NULL) == -1)
    exit_perror_log(SIG_FAIL, "setting SIGINT handler failed.");
}

void set_pg_id(pid_t pid) { pg_id = pid; }

pid_t get_pg_id(void) { return pg_id; }
