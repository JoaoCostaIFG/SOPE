#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "include/init.h"
#include "include/logs.h"
#include "include/sigs.h"

#define STAT_DFLT_SIZE 512
#define MAX_CHILDREN 500

#define READ 0
#define WRITE 1

static size_t child_num = 0;

struct child_elem {
  pid_t pid; // if > 0 pid o process, if -1 process dead
  int fd[2];
};
typedef struct child_elem child_elem;

static child_elem children[MAX_CHILDREN];
static int parent_pipe[2];
static unsigned long my_size;
static cmd_opt cmd_opts;

void pipe_send() {
  if (is_child()) {
    // write pipe
    FILE *fp;
    if ((fp = fdopen(parent_pipe[WRITE], "w")) == NULL)
      exit_perror_log(FILE_OPEN_ERROR, "Failure opening file");
    if (fprintf(fp, "%lu\n", my_size) < 0)
      exit_perror_log(PIPE_FAIL, "Writing to pipe failure");
    fclose(fp);
    write_sendpipe_log(my_size); // log pipe send

    if (cmd_opts.max_depth != 0) { // depth = 0 => last level
      printf("%lu\t%s\n", my_size, cmd_opts.path);
      fflush(stdout);
    }
  } else {
    my_size += cmd_opts.bytes
                   ? DIR_NUM_BLK * DFLT_BLK_SIZE
                   : DIR_NUM_BLK * DFLT_BLK_SIZE / cmd_opts.block_size;
    printf("%lu\t%s\n", my_size, cmd_opts.path);
    fflush(stdout);
  }

  write_entry_log(my_size, cmd_opts.path);
}

void rm_child(pid_t pid) {
  if (pid < 0)
    return;

  unsigned long size = 0;
  for (size_t i = 0; i < child_num; ++i) {
    if (children[i].pid == pid) {
      children[i].pid = -1;

      // read pipe info
      FILE *fp;
      if ((fp = fdopen(children[i].fd[READ], "r")) == NULL)
        exit_perror_log(FILE_OPEN_ERROR, "Failure opening file");
      if (fscanf(fp, "%lu", &size) == EOF)
        exit_perror_log(PIPE_FAIL, "Reading from pipe failure");
      fclose(fp);
      close(children[i].fd[READ]); // close fd
      write_sendpipe_log(my_size); // log pipe receive

      if (!cmd_opts.separate_dirs) // skip sub-dir size
        my_size += size;
      return;
    }
  }
}

void child_reaper(int reap_all) {
  if (!child_num)
    return;

  if (reap_all) {
    for (size_t i = 0; i < child_num; ++i) {
      rm_child(wait(NULL));
    }
  } else {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
      rm_child(pid);
    }
  }
}

char *assemble_args(char *argv0) {
  /* args normais, B, depth, path, terminating NULL */
  char *args = (char *)malloc(sizeof(char) * MAX_PATH_SIZE);
  if (!args)
    exit_log(MALLOC_FAIL);

  strcpy(args, argv0);

  char tmp[40];
  sprintf(tmp, "-%s%s%s%s%s", cmd_opts.all ? "a" : "",
          cmd_opts.bytes ? "b" : "", cmd_opts.count_links ? "l" : "",
          cmd_opts.dereference ? "L" : "", cmd_opts.separate_dirs ? "S" : "");
  if (strcmp(tmp, "-")) {
    strcat(args, " ");
    strcat(args, tmp);
  }

  if (cmd_opts.block_size != 1024) {
    strcat(args, " ");
    sprintf(tmp, "-B=%d", cmd_opts.block_size);
    strcat(args, tmp);
  }

  if (cmd_opts.max_depth >= 0) {
    strcat(args, " ");
    sprintf(tmp, "--max-depth=%d", cmd_opts.max_depth);
    strcat(args, tmp);
  } else if (cmd_opts.max_depth == -2) {
    strcat(args, " --max_depth=0");
  }

  /* path */
  strcat(args, " ");
  strcat(args, cmd_opts.path);
  return args;
}

void read_files() {
  DIR *dirp;
  if (!(dirp = opendir(cmd_opts.path))) {
    perror(cmd_opts.path);
    exit_perror_log(FAILED_OPENDIR, cmd_opts.path);
  }

  struct stat stat_buf;
  unsigned long size;
  struct dirent *direntp;
  char path[MAX_PATH_SIZE];
  while ((direntp = readdir(dirp))) {
    /* skip "." && ".." */
    if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, ".."))
      continue;

    child_reaper(0); // reap children

    /* get formatted file path */
    pathcpycat(path, cmd_opts.path, direntp->d_name);
    if ((cmd_opts.dereference ? stat(path, &stat_buf)
                              : lstat(path, &stat_buf)) == -1)
      perror(path);

    if (S_ISREG(stat_buf.st_mode) || S_ISLNK(stat_buf.st_mode)) {
      size = cmd_opts.bytes
                 ? stat_buf.st_size
                 : stat_buf.st_blocks * STAT_DFLT_SIZE / cmd_opts.block_size;
      my_size += size;

      if (cmd_opts.all && cmd_opts.max_depth != 0 &&
          cmd_opts.max_depth != -2) { // depth = 0 => last level
        if (!cmd_opts.bytes) {
          if (stat_buf.st_blocks == 0)
            size = 0;
          else {
            size = stat_buf.st_blocks * STAT_DFLT_SIZE / cmd_opts.block_size;
            if (size == 0)
              size = 1;
          }
        }
        printf("%lu\t%s\n", size, path);
        fflush(stdout);
      }
      write_entry_log(size, path);
    }
  }

  closedir(dirp);
}

int read_dirs(char *argv0) {
  DIR *dirp;
  if (!(dirp = opendir(cmd_opts.path))) {
    perror(cmd_opts.path);
    exit_perror_log(FAILED_OPENDIR, cmd_opts.path);
  }

  struct stat stat_buf;
  struct dirent *direntp;
  char path[MAX_PATH_SIZE];
  while ((direntp = readdir(dirp))) {
    /* skip "." && ".." */
    if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, ".."))
      continue;

    child_reaper(0); // reap children

    /* get formatted file path */
    pathcpycat(path, cmd_opts.path, direntp->d_name);
    if ((cmd_opts.dereference ? stat(path, &stat_buf)
                              : lstat(path, &stat_buf)) == -1)
      perror(path);

    if (S_ISDIR(stat_buf.st_mode)) {

      char *tmp;
      // pipe
      pipe(children[child_num].fd);
      children[child_num].pid = fork();
      switch (children[child_num].pid) {
      case -1: // failed fork
        exit_perror_log(FORK_FAIL, "");
        break;
      case 0: // child
        // cmd line args
        strcpy(cmd_opts.path, path);
        tmp = assemble_args(argv0);
        LOG_CREATE(tmp);
        free(tmp);

        set_signals();

        // save parent pipe
        parent_pipe[READ] = children[child_num].fd[READ];
        parent_pipe[WRITE] = children[child_num].fd[WRITE];
        close(parent_pipe[READ]); // close reading end

        child_num = 0; // reset child count
        my_size = cmd_opts.bytes ? stat_buf.st_size
                                 : stat_buf.st_blocks * STAT_DFLT_SIZE /
                                       cmd_opts.block_size;

        if (cmd_opts.max_depth == 1)
          cmd_opts.max_depth = -2; // print only the dir
        else if (cmd_opts.max_depth == -2)
          cmd_opts.max_depth = 0; // don't print anything else
        else if (cmd_opts.max_depth > 0)
          --cmd_opts.max_depth; // lower one lvl if not -1 (infinite)

        return 1; // repeat
        break;
      default:                                // parent
        close(children[child_num].fd[WRITE]); // close writing end
        ++child_num;
        break;
      }
    }
  }

  closedir(dirp);
  return 0;
}

void path_handler(char *argv0) {
  /* test if file was given, and handle it */
  struct stat stat_buf;
  if ((cmd_opts.dereference ? stat(cmd_opts.path, &stat_buf)
                            : lstat(cmd_opts.path, &stat_buf)) == -1)
    exit_perror_log(NON_EXISTING_ENTRY, cmd_opts.path);
  if (!S_ISDIR(stat_buf.st_mode)) {
    printf("%lu\t%s\n",
           cmd_opts.bytes
               ? stat_buf.st_size
               : stat_buf.st_blocks * STAT_DFLT_SIZE / cmd_opts.block_size,
           cmd_opts.path);
    exit_log(EXIT_SUCCESS);
  }

  do {
    if (read_dirs(argv0)) // fork dirs
      continue;
    read_files(); // handle files
    break;
  } while (1);

  child_reaper(1);
  pipe_send();
}

int main(int argc, char *argv[]) {
  init(argc, argv, &cmd_opts);
  set_signals();

  path_handler(argv[0]); // fork subdirs and process files
  exit_log(EXIT_SUCCESS);
}
