#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "include/logs.h"

static char log_file[MAX_PATH_SIZE] = LOG_FILE;

void write_log(char* action, char *info) {
  FILE * fp = fopen(log_file, "a");
  fprintf(fp, "%.2f - %d - %s - %s\n", (float)clock()/CLOCKS_PER_SEC * 1000, getpid(), "CREATE", info);
  fclose(fp);
}

void set_logfile(char *new_logfile) {
  if (new_logfile)
    strncpy(log_file, new_logfile, MAX_PATH_SIZE);
}

void clrlogs(void) {
  /* clear log file contents */
  fclose(fopen(LOG_FILE, "w+"));
}
