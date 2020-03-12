#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "include/logs.h"

#define LOG_ENV_NAME "LOG_FILENAME"

static 

void init(char **envp) {
  set_logfile(getenv(LOG_ENV_NAME));
}

int main(int argc, char *argv[], char *envp[]) {
  init(envp);
  return 0;
}
