#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "include/logs.h"

static char log_file[MAX_LOG_PATH_SIZE + 1] = LOG_DIR LOG_FILE;

// TODO need to fix the time
// it isn't getting the time since parent start (it gets since its own start)
void write_log(char *action, char *info) {
  FILE *fp = fopen(log_file, "a");
  fprintf(fp, "%.2f - %d - %s - %s\n", (float)clock() / CLOCKS_PER_SEC * 1000,
          getpid(), action, info);
  fclose(fp);
}

void write_entry_log(unsigned size, char *name) {
  char info[MAX_LOG_PATH_SIZE + 1];
  sprintf(info, "%u - %s", size, name);

  LOG_ENTRY(info);
}

void write_create_log(int argc, char **argv) {
  FILE *fp = fopen(log_file, "a");

  fprintf(fp, "%.2f - %d - %s -", (float)clock() / CLOCKS_PER_SEC * 1000,
          getpid(), CREATE_LOG);

  for (int i = 0; i < argc; ++i)
    fprintf(fp, " %s", argv[i]);
  fputc('\n', fp);

  fclose(fp);
}

void write_recvpipe_log(long info) {
  char str[256];
  sprintf(str, "%ld", info);
  LOG_RECVPIPE(str);
}

void write_sendpipe_log(long info) {
  char str[256];
  sprintf(str, "%ld", info);
  LOG_SENDPIPE(str);
}

static char *signum2signal(int signum) {
  switch (signum) {
  case SIGCONT:
    return SIGCONT_TEXT;
    break;
  case SIGINT:
    return SIGINT_TEXT;
    break;
  case SIGSTOP:
    return SIGSTOP_TEXT;
    break;
  case SIGTERM:
    return SIGTERM_TEXT;
    break;
  default:
    return "";
    break;
  }
}

void write_recvsig_log(int signum) { LOG_RECVSIG(signum2signal(signum)); }

void write_sendsig_log(int signum, pid_t pid) {
  char info[256];
  sprintf(info, "%s - %d", signum2signal(signum), pid);
  LOG_SENDSIG(info);
}

void exit_log(int exit_code) {
  char info[16];
  sprintf(info, "%d", exit_code);

  LOG_EXIT(info);
  exit(exit_code);
}

void exit_perror_log(int exit_code, char *msg) {
  perror(msg);
  exit_log(exit_code);
}

void exit_err_log(int exit_code, char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit_log(exit_code);
}

void set_logfile(char *new_logfile) {
  if (new_logfile) {
    strcpy(log_file, LOG_DIR);
    strncpy(log_file, new_logfile, MAX_LOG_PATH_SIZE);
  }
}

void clrlogs(void) {
  /* clear log file contents */
  fclose(fopen(log_file, "w+"));
}
