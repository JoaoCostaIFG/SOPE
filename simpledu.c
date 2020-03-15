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
  killpg(-pg_id, SIGSTOP);
  // get user choice
  char ans;
  do {
    printf("Do you really wish to stop execution? [y/n] ");
  } while (scanf("%c", &ans) != EOF &&
           (tolower(ans) != 'y' && tolower(ans) != 'n' && ans != '\n'));
  // process decision
  if (tolower(ans) == 'n') { // unpause children
    killpg(-pg_id, SIGCONT);
  } else { // kill children
    killpg(-pg_id, SIGTERM);
    exit_log(EXIT_SUCCESS);
  }
}

static void set_child_sig(void) {
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
  pg_id = getpgid(0); // save process group id for children
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
    waitpid(-1, NULL, WNOHANG);
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
        /*
         * // exec
         * execv(pname, assemble_args(pname, cmd_opts, path));
         * exit_perror_log(EXEC_FAIL, pname);
         */
        set_child_sig();
        child_num = 0;
        if (cmd_opts->max_depth > 0)
          --cmd_opts->max_depth;
        strcpy(cmd_opts->path, path);
        return 1;
        break;
      default: // parent
        ++child_num;
        break;
      }
    } else if (!dir_run && cmd_opts->all &&
               (S_ISREG(stat_buf.st_mode) || S_ISLNK(stat_buf.st_mode)))
      printf("%lu\t%s\n", size, path);
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
    read_dir_files(cmd_opts, 0);
  }

  child_reaper(1); // read all processes
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

/* static char **assemble_args(char *pname, cmd_opt *cmd_opts, char *file_path) { */
  /* [> args normais, B, depth, path, terminating NULL <] */
  /* char **args = (char **)malloc(sizeof(char *) * ARGS_MTR_SIZE); */
  /* if (!args) */
    /* exit_err_log(MALLOC_FAIL, "Heap memory allocation failure."); */

  /* [> save program name (argv[0]) <] */
  /* if (!(args[0] = (char *)malloc(sizeof(char) * (strlen(pname) + 1)))) */
    /* exit_err_log(MALLOC_FAIL, "Heap memory allocation failure."); */
  /* strcpy(args[0], pname); */

  /* [> args <] */
  /* int i = 1; // curr arg postion */
  /* args[i] = malloc(sizeof(char) * 7); */
  /* sprintf(args[i++], "-%s%s%s%s%s", cmd_opts->all ? "a" : "", */
          /* cmd_opts->bytes ? "b" : "", cmd_opts->count_links ? "l" : "", */
          /* cmd_opts->dereference ? "L" : "", cmd_opts->separate_dirs ? "S" : ""); */
  /* if (!strcmp(args[1], "-")) */
    /* free(args[1]); */

  /* if (cmd_opts->block_size != 1024) { */
    /* if (!(args[i] = (char *)malloc(sizeof(char) * 16))) */
      /* exit_err_log(MALLOC_FAIL, "Heap memory allocation failure."); */
    /* sprintf(args[i++], "-B=%d", cmd_opts->block_size); */
  /* } */

  /* if (cmd_opts->max_depth != -1) { // decrease max-depth */
    /* if (!(args[i] = (char *)malloc(sizeof(char) * 24))) */
      /* exit_err_log(MALLOC_FAIL, "Heap memory allocation failure."); */
    /* sprintf(args[i++], "--max-depth=%d", cmd_opts->max_depth - 1); */
  /* } */

  /* [> path <] */
  /* if (!(args[i] = (char *)malloc(sizeof(char) * (strlen(file_path) + 1)))) */
    /* exit_err_log(MALLOC_FAIL, "Heap memory allocation failure."); */
  /* strcpy(args[i++], file_path); */

  /* args[i] = (char *)0; */
  /* return args; */
/* } */
