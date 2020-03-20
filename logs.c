#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "include/logs.h"

#define TIMEREF_S_ENV "SIMPLEDU_S_TIMEREF"
#define TIMEREF_NS_ENV "SIMPLEDU_NS_TIMEREF"

#define MAX_TIME_LEN 30

#define NANOSINSEC 1000000000
#define MILISINNANO 1000000

static char log_file[MAX_LOG_PATH_SIZE + 1] = LOG_DIR LOG_FILE;
static struct timespec tm;

double tm_to_milisecond(struct timespec *tm_now) {
  double ret = (double)tm_now->tv_sec * NANOSINSEC + tm_now->tv_nsec;
  ret -= tm.tv_sec * NANOSINSEC + tm.tv_nsec;
  ret /= MILISINNANO;

  return ret;
}

void write_log(char *action, char *info) {
  FILE *fp;
  if ((fp = fopen(log_file, "a")) == NULL) {
    perror("Log file opening error");
    exit(FILE_OPEN_ERROR);
  }

  struct timespec tm_now;
  if (clock_gettime(CLOCK_MONOTONIC, &tm_now) == -1) {
    perror("Couldn't get time.");
    exit(TIME_ERROR);
  }

  fprintf(fp, "%.2lf - %d - %s - %s\n", tm_to_milisecond(&tm_now), getpid(),
          action, info);
  fclose(fp);
}

void write_entry_log(unsigned size, char *name) {
  char info[MAX_LOG_PATH_SIZE + 1];
  sprintf(info, "%u - %s", size, name);

  LOG_ENTRY(info);
}

void write_create_log(char **argv) {
  FILE *fp = fopen(log_file, "a");

  struct timespec tm_now;
  if (clock_gettime(CLOCK_MONOTONIC, &tm_now) == -1) {
    perror("Couldn't get time.");
    exit(TIME_ERROR);
  }

  fprintf(fp, "%.2lf - %d - %s -", tm_to_milisecond(&tm_now), getpid(),
          CREATE_LOG);

  for (int i = 0; argv[i] != NULL; ++i)
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

void save_starttime(void) {
  if (clock_gettime(CLOCK_MONOTONIC, &tm) == -1) {
    perror("Couldn't get program start time.");
    exit(TIME_ERROR);
  }

  // TODO error check
  char sec[MAX_TIME_LEN + 1], nanosec[MAX_TIME_LEN + 1];
  snprintf(sec, 30, "%lld", (long long)tm.tv_sec);
  snprintf(nanosec, 30, "%ld", tm.tv_nsec);

  setenv(TIMEREF_S_ENV, sec, 1);
  setenv(TIMEREF_NS_ENV, nanosec, 1);
}

void get_reftime(void) {
  char *tmp_sec, *tmp_nsec;
  if ((tmp_sec = getenv(TIMEREF_S_ENV)) == NULL ||
      (tmp_nsec = getenv(TIMEREF_NS_ENV)) == NULL) {
    save_starttime();
    return;
  }

  tm.tv_sec = strtoll(tmp_sec, NULL, 10);
  tm.tv_nsec = strtol(tmp_nsec, NULL, 10);
}
