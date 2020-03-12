#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "include/logs.h"

static char log_file[MAX_PATH_SIZE + 6] = LOG_DIR LOG_FILE;

// TODO need to fix the time
// it isn't getting the time since parent start (it gets since its own start)
void write_log(char *action, char *info) {
  FILE *fp = fopen(log_file, "a");
  fprintf(fp, "%.2f - %d - %s - %s\n", (float)clock() / CLOCKS_PER_SEC * 1000,
          getpid(), action, info);
  fclose(fp);
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

void exit_log(int exit_code) {
  char info[16];
  sprintf(info, "%d", exit_code);

  LOG_EXIT(info);
  exit(exit_code);
}

void set_logfile(char *new_logfile) {
  if (new_logfile) {
    strcpy(log_file, LOG_DIR);
    strncpy(log_file + 5, new_logfile, MAX_PATH_SIZE);
  }
}

void clrlogs(void) {
  /* clear log file contents */
  fclose(fopen(LOG_FILE, "w"));
}
