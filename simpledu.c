#include <ctype.h>
#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "include/init.h"
#include "include/logs.h"

#define STAT_DFLT_SIZE 512
#define ARGS_MTR_SIZE 6

static pid_t child_pids[256];
static size_t child_num = 0;
static pid_t pg_id; /**< process group of children */

static void sigint_handler(int signum) {
  if (signum != SIGINT)
    return;

  // pause children
  killpg(pg_id, SIGSTOP);
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
    killpg(pg_id, SIGCONT);
  } else { // kill children
    puts("Exiting..");
    killpg(pg_id, SIGTERM);
    exit_log(EXIT_SUCCESS);
  }
}

static void set_child_sig(void) {
  if (getpgrp() == pg_id) // permission already set
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

static void set_grandparent_sig(void) {
  pg_id = getpid() + 1; // save process group id for children
  /* handle SIGINT */
  struct sigaction sa;
  sa.sa_handler = &sigint_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_NOCLDSTOP;
  if (sigaction(SIGINT, &sa, NULL) == -1)
    exit_perror_log(SIG_FAIL, "setting SIGINT handler failed.");
}

void child_reaper(int reap_all) {
  if (reap_all) {
    for (size_t i = 0; i < child_num; ++i) {
      wait(NULL);
    }
  } else {
    if (child_num && waitpid(-1, NULL, WNOHANG) > 0)
      --child_num;
  }
}

int read_dir_files(cmd_opt *cmd_opts, int dir_run) {
  DIR *dirp;
  if (!(dirp = opendir(cmd_opts->path))) {
    perror(cmd_opts->path);
    exit_perror_log(FAILED_OPENDIR, cmd_opts->path);
  }

  struct stat stat_buf;
  unsigned long size = 0;
  struct dirent *direntp;
  char path[MAX_PATH_SIZE];
  while ((direntp = readdir(dirp))) {
    if (!strcmp(direntp->d_name, ".") ||
        !strcmp(direntp->d_name, "..")) // skip "." && ".."
      continue;

    child_reaper(0); // reap children

    /* get formatted file path */
    pathcpycat(path, cmd_opts->path, direntp->d_name);
    if ((cmd_opts->dereference ? stat(path, &stat_buf)
                               : lstat(path, &stat_buf)) == -1)
      exit_perror_log(NON_EXISTING_ENTRY, path);

    size = cmd_opts->bytes
               ? stat_buf.st_size
               : stat_buf.st_blocks * STAT_DFLT_SIZE / cmd_opts->block_size;

    if (S_ISDIR(stat_buf.st_mode) && dir_run) {
      printf("%lu\t%s\n", size, path);
      child_pids[child_num] = fork();
      switch (child_pids[child_num]) {
      case -1: // failed fork
        exit_perror_log(FORK_FAIL, "");
        break;
      case 0: // child
        set_child_sig();
        child_num = 0;
        if (cmd_opts->max_depth > 0)
          --cmd_opts->max_depth;
        strcpy(cmd_opts->path, path);
        return 1; // repeat
        break;
      default: // parent
        ++child_num;
        break;
      }
    } else if (!dir_run && cmd_opts->all &&
               (S_ISREG(stat_buf.st_mode) || S_ISLNK(stat_buf.st_mode)))
      printf("%lu\t%s\n", size, path);

    fflush(stdout);
  }

  closedir(dirp);
  return 0;
}

void path_handler(cmd_opt *cmd_opts) {
  /* test if file was given, and handle it */
  struct stat stat_buf;
  if ((cmd_opts->dereference ? stat(cmd_opts->path, &stat_buf)
                             : lstat(cmd_opts->path, &stat_buf)) == -1)
    exit_perror_log(NON_EXISTING_ENTRY, cmd_opts->path);
  if (!S_ISDIR(stat_buf.st_mode)) {
    printf("%lu\t%s\n",
           cmd_opts->bytes
               ? stat_buf.st_size
               : stat_buf.st_blocks * STAT_DFLT_SIZE / cmd_opts->block_size,
           cmd_opts->path);
    exit_log(EXIT_SUCCESS);
  }

  int repeat = 1;
  while (repeat) {
    /* fork dirs */
    if ((repeat = read_dir_files(cmd_opts, 1)))
      continue;
    /* handle files */
    repeat = read_dir_files(cmd_opts, 0);
  }

  child_reaper(1); // reap all processes
}

int main(int argc, char *argv[]) {
  cmd_opt cmd_opts;
  init(argc, argv, &cmd_opts);
  set_grandparent_sig();

  // depth = 0 => dizer apenas tamanho atual da dir
  if (cmd_opts.max_depth == 0) // max depth reached, quit
    exit_log(0);

  path_handler(&cmd_opts); // fork subdirs and process files
  exit_log(EXIT_SUCCESS);
}
