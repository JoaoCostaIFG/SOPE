#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "include/logs.h"
#include "include/sigs.h"
#include "include/utls.h"

#define GRPID_ENV "SIMPLEDU_GRPID"

#define SIGINT_ANS_LEN 200
#define SIGINT_HAND_MSG "Do you really wish to stop execution? [y/n] "
#define SIGINT_HAND_MSG_LEN 45
#define SIGINT_HAND_CONT "Continue..\n"
#define SIGINT_HAND_CONT_LEN 11
#define SIGINT_HAND_EXIT "Exitting..\n"
#define SIGINT_HAND_EXIT_LEN 11

static pid_t pg_id = 0; /**< process group of children (0 is invalid) */

pid_t getGrpId(void) {
  if (get_env_int(GRPID_ENV, &pg_id))
    pg_id = 0;

  return pg_id;
}

void set_pg_id(pid_t pid) {
  pg_id = pid;
  set_env_int(GRPID_ENV, pid, 1);
}

void set_grandparent(void) { set_pg_id(getpid() + 1); }

int is_child(void) { return (getpid() + 1 != pg_id); }

int is_grandparent(void) { return (getpid() + 1 == pg_id || pg_id == 0); }

/* garantee reiterant funcs only */
void sigint_handler(int signum) {
  if (signum != SIGINT)
    return;
  // pause children
  kill(-pg_id, SIGSTOP);

  // logs
  write_recvsig_log(SIGINT);
  write_sendsig_log(SIGSTOP, -pg_id);

  // get user choice
  write(STDERR_FILENO, "\n", 1);
  char ans[SIGINT_ANS_LEN + 1];
  int read_ret;
  do {
    // loop until either 'n', 'N', 'y' or 'Y' is read from stdin
    write(STDERR_FILENO, SIGINT_HAND_MSG, SIGINT_HAND_MSG_LEN);
    read_ret = read(STDIN_FILENO, ans, SIGINT_ANS_LEN);
    if (read_ret == -1) { // if read fails, assume yes
      ans[0] = 'y';
      read_ret = 0;
    } else if (read_ret <= 2 && ans[1] == '\n') {
      read_ret = 0;
    }
  } while (read_ret != 0);

  // process decision
  if (ans[0] == 'n' || ans[0] == 'N') { // unpause children
    write(STDERR_FILENO, SIGINT_HAND_CONT, SIGINT_HAND_CONT_LEN);
    kill(-pg_id, SIGCONT);
    write_sendsig_log(SIGCONT, -pg_id);
  } else { // kill children
    write(STDERR_FILENO, SIGINT_HAND_EXIT, SIGINT_HAND_EXIT_LEN);
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
