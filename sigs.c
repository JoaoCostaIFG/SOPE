#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "include/logs.h"
#include "include/sigs.h"

#define GRPID_ENV "SIMPLEDU_GRPID"

static pid_t pg_id = 0; /**< process group of children (0 is invalid) */

pid_t getGrpId(void) {
  char *tmp;
  if ((tmp = getenv(GRPID_ENV)) == NULL) {
    pg_id = 0;
    return 0;
  }

  pg_id = (pid_t)atol(tmp);
  return pg_id;
}

void set_pg_id(pid_t pid) {
  pg_id = pid;
  char env_var[20];
  sprintf(env_var, "%d", pg_id);

  setenv(GRPID_ENV, env_var, 1);
}

void set_grandparent(void) { set_pg_id(getpid() + 1); }

int is_child(void) { return (getpid() + 1 != pg_id); }

int is_grandparent(void) { return (getpid() + 1 == pg_id || pg_id == 0); }

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
    fputs("Do you really wish to stop execution? [y/n] ", stderr);
    fflush(stderr);
  } while ((ans = getchar()) != EOF &&
           (tolower(ans) != 'y' && tolower(ans) != 'n'));
  getchar();

  // process decision
  if (tolower(ans) == 'n') { // unpause children
    fputs("Continue..\n", stderr);
    fflush(stderr);
    kill(-pg_id, SIGCONT);
    write_sendsig_log(SIGCONT, -pg_id);
  } else { // kill children
    fputs("Exiting..\n", stderr);
    fflush(stderr);
    kill(-pg_id, SIGTERM);
    write_sendsig_log(SIGTERM, -pg_id);
    exit_log(EXIT_SUCCESS);
  }
}

void set_child_sig(void) {
  /* ignore SIGINT */
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGINT, &sa, NULL) == -1)
    exit_perror_log(SIG_FAIL, "ignoring SIGINT handler failed.");
}

void set_grandparent_sig(void) {
  /* handle SIGINT */
  struct sigaction sa;
  sa.sa_handler = &sigint_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGINT, &sa, NULL) == -1)
    exit_perror_log(SIG_FAIL, "setting SIGINT handler failed.");
}

void set_signals(void) {
  getGrpId();
  if (is_grandparent()) {
    set_grandparent(); // save process group id for children
    set_grandparent_sig();
  } else {
    setpgid(0, pg_id); // set process group id
    set_child_sig();
  }
}
