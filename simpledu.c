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
#define ARGS_MTR_SIZE 6

#define READ 0
#define WRITE 1

static size_t child_num = 0;

struct child_elem { // TODO careful com signess de pid_t
  pid_t pid;        // if > 0 pid o process, if <= 0 size of dir
  int fd[2];
};
typedef struct child_elem child_elem;
static child_elem children[256];
static int parent_pipe[2];
static unsigned my_size;

void pipe_send(cmd_opt *cmd_opts) {
  if (getpid() + 1 != get_pg_id()) {
    // write pipe
    FILE *fp;
    if ((fp = fdopen(parent_pipe[WRITE], "w")) == NULL)
      exit_perror_log(FILE_OPEN_ERROR, "Failure opening file");
    if (fprintf(fp, "%u\n", my_size) < 0)
      exit_perror_log(PIPE_FAIL, "Writing to pipe failure");
    fclose(fp);
    // log pipe send // TODO
    char str[200];
    sprintf(str, "%u", my_size);
    LOG_SENDPIPE(str);
  } else {
    my_size += cmd_opts->bytes
                   ? DIR_NUM_BLK * DFLT_BLK_SIZE
                   : DIR_NUM_BLK * DFLT_BLK_SIZE / cmd_opts->block_size;
  }

  printf("%u\t%s\n", my_size, cmd_opts->path);
  fflush(stdout);
}

void rm_child(pid_t pid) {
  if (pid < 0)
    return;

  unsigned size = 0;
  for (size_t i = 0; i < child_num; ++i) {
    if (children[i].pid == pid) {
      // read pipe info
      FILE *fp;
      if ((fp = fdopen(children[i].fd[READ], "r")) == NULL)
        exit_perror_log(FILE_OPEN_ERROR, "Failure opening file");
      if (fscanf(fp, "%u", &size) == EOF)
        exit_perror_log(PIPE_FAIL, "Reading from pipe failure");
      fclose(fp);
      // log pipe receise // TODO
      char str[200];
      sprintf(str, "%u", size);
      LOG_RECVPIPE(str);

      children[i].pid = -1;

      /* if (!cmd_opts->separate_dirs) // skip sub-dir size */ // TODO
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

// TODO LOG ENTRYS
void read_files(cmd_opt *cmd_opts) {
  DIR *dirp;
  if (!(dirp = opendir(cmd_opts->path))) {
    perror(cmd_opts->path);
    exit_perror_log(FAILED_OPENDIR, cmd_opts->path);
  }

  struct stat stat_buf;
  unsigned size = 0;
  struct dirent *direntp;
  char path[MAX_PATH_SIZE];
  while ((direntp = readdir(dirp))) {
    /* skip "." && ".." */
    if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, ".."))
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

    if (cmd_opts->all &&
        (S_ISREG(stat_buf.st_mode) || S_ISLNK(stat_buf.st_mode))) {
      my_size += size;
      printf("%u\t%s\n", size, path);
      fflush(stdout);
    }
  }

  closedir(dirp);
}

int read_dirs(cmd_opt *cmd_opts) {
  DIR *dirp;
  if (!(dirp = opendir(cmd_opts->path))) {
    perror(cmd_opts->path);
    exit_perror_log(FAILED_OPENDIR, cmd_opts->path);
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
    pathcpycat(path, cmd_opts->path, direntp->d_name);
    if ((cmd_opts->dereference ? stat(path, &stat_buf)
                               : lstat(path, &stat_buf)) == -1)
      exit_perror_log(NON_EXISTING_ENTRY, path);

    if (S_ISDIR(stat_buf.st_mode)) {
      LOG_ENTRY(path);

      // pipe
      pipe(children[child_num].fd);
      children[child_num].pid = fork();
      switch (children[child_num].pid) {
      case -1: // failed fork
        exit_perror_log(FORK_FAIL, "");
        break;
      case 0:             // child
        LOG_CREATE(path); // TODO
        set_child_sig();

        // save parent pipe
        parent_pipe[READ] = children[child_num].fd[READ];
        parent_pipe[WRITE] = children[child_num].fd[WRITE];
        close(parent_pipe[READ]); // close reading end
        child_num = 0;            // reset child count

        my_size = cmd_opts->bytes ? stat_buf.st_size
                                  : stat_buf.st_blocks * STAT_DFLT_SIZE /
                                        cmd_opts->block_size;
        if (cmd_opts->max_depth > 0)
          --cmd_opts->max_depth;
        strcpy(cmd_opts->path, path);
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

  do {
    if (read_dirs(cmd_opts)) // fork dirs
      continue;
    read_files(cmd_opts); // handle files
    break;
  } while (1);

  child_reaper(1);
  pipe_send(cmd_opts);
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
