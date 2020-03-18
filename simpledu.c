#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <tgmath.h>
#include <unistd.h>

#include "include/init.h"
#include "include/logs.h"
#include "include/sigs.h"

#define STAT_DFLT_SIZE 512
#define MAX_CHILDREN 500

#define READ 0
#define WRITE 1

struct child_elem {
  pid_t pid; // if > 0 pid o process, if -1 process dead
  int fd[2];
};
typedef struct child_elem child_elem;

static child_elem children[MAX_CHILDREN];
static unsigned long my_size;
static prog_prop prog_props;

unsigned long lu_ceil(double d) { return (unsigned long)ceil(d); }

unsigned long calc_size(struct stat *stat_buf) {
  return prog_props.bytes ? stat_buf->st_size
                          : stat_buf->st_blocks * STAT_DFLT_SIZE;
}

void pipe_send() {
  if (is_child()) {
    // write pipe
    FILE *fp;
    if ((fp = fdopen(prog_props.parent_pipe[WRITE], "w")) == NULL)
      exit_perror_log(FILE_OPEN_ERROR, "Failure opening file");
    if (fprintf(fp, "%lu\n", my_size) < 0)
      exit_perror_log(PIPE_FAIL, "Writing to pipe failure");
    fclose(fp);
    write_sendpipe_log(my_size); // log pipe send

    if (prog_props.max_depth != 0) { // depth = 0 => last level
      printf("%lu\t%s\n", lu_ceil((double)my_size / prog_props.block_size),
             prog_props.path);
    }
  } else {
    printf("%lu\t%s\n", lu_ceil((double)my_size / prog_props.block_size),
           prog_props.path);
  }

  fflush(stdout);
  write_entry_log(my_size, prog_props.path);
}

void rm_child(pid_t pid) {
  if (pid < 0)
    return;

  unsigned long size = 0;
  for (size_t i = 0; i < prog_props.child_num; ++i) {
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

      if (!prog_props.separate_dirs) // skip sub-dir size
        my_size += size;
      return;
    }
  }
}

void child_reaper(int reap_all) {
  if (!prog_props.child_num)
    return;

  if (reap_all) {
    for (size_t i = 0; i < prog_props.child_num; ++i) {
      rm_child(wait(NULL));
    }
  } else {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
      rm_child(pid);
    }
  }
}

void read_files(DIR *dirp) {
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
    pathcpycat(path, prog_props.path, direntp->d_name);
    if ((prog_props.dereference ? stat(path, &stat_buf)
                                : lstat(path, &stat_buf)) < 0)
      perror(path);

    if (S_ISREG(stat_buf.st_mode) || S_ISLNK(stat_buf.st_mode)) {
      size = calc_size(&stat_buf);
      my_size += size;

      if (prog_props.all && prog_props.max_depth != 0 &&
          prog_props.max_depth != -2) { // depth = 0 => last level
        printf("%lu\t%s\n", lu_ceil((double)size / prog_props.block_size),
               path);
        fflush(stdout);
      }
      // TODO acompanhar flags or sempre tamanho real ?
      write_entry_log(size, path);
    }
  }
}

int read_dirs(DIR *dirp, char *argv0) {
  struct stat stat_buf;
  struct dirent *direntp;
  char path[MAX_PATH_SIZE];
  while ((direntp = readdir(dirp))) {
    /* skip "." && ".." */
    if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, ".."))
      continue;

    child_reaper(0); // reap children

    /* get formatted file path */
    pathcpycat(path, prog_props.path, direntp->d_name);
    if ((prog_props.dereference ? stat(path, &stat_buf)
                                : lstat(path, &stat_buf)) < 0)
      perror(path);

    if (S_ISDIR(stat_buf.st_mode)) {
      // pipe
      pipe(children[prog_props.child_num].fd);
      children[prog_props.child_num].pid = fork();
      switch (children[prog_props.child_num].pid) {
      case -1: // failed fork
        exit_perror_log(FORK_FAIL, "");
        break;
      case 0: // child
        init_child(argv0, path, &prog_props);

        // save parent pipe
        prog_props.parent_pipe[READ] = children[prog_props.child_num].fd[READ];
        prog_props.parent_pipe[WRITE] =
            children[prog_props.child_num].fd[WRITE];
        close(prog_props.parent_pipe[READ]); // close reading end

        prog_props.child_num = 0; // reset child count
        my_size = 0;

        return 1; // repeat
        break;
      default:                                           // parent
        close(children[prog_props.child_num].fd[WRITE]); // close writing end
        ++prog_props.child_num;
        break;
      }
    }
  }
  return 0;
}

void path_handler(char *argv0) {
  /* test if file was given, and handle it */
  struct stat stat_buf;
  if ((prog_props.dereference ? stat(prog_props.path, &stat_buf)
                              : lstat(prog_props.path, &stat_buf)) == -1)
    exit_perror_log(NON_EXISTING_ENTRY, prog_props.path);
  if (!S_ISDIR(stat_buf.st_mode)) {
    my_size = calc_size(&stat_buf);
    printf("%lu\t%s\n", lu_ceil((double)my_size / prog_props.block_size),
           prog_props.path);
    exit_log(EXIT_SUCCESS);
  }
  /*
   * else {
   *   my_size = calc_size(&stat_buf);
   * }
   */

  /* handle dires and their files/links */
  DIR *dirp;
  int analyse = 1;
  while (analyse) {
    if ((prog_props.dereference ? stat(prog_props.path, &stat_buf)
                                : lstat(prog_props.path, &stat_buf)) < 0) {
      pipe_send();
      exit_perror_log(STAT_FAIL, prog_props.path);
    }
    my_size = calc_size(&stat_buf);

    if (!(dirp = opendir(prog_props.path))) {
      pipe_send();
      exit_perror_log(FAILED_OPENDIR, prog_props.path);
    }

    if (read_dirs(dirp, argv0)) { // fork dirs
      analyse = 1;
      closedir(dirp);
      continue;
    }

    analyse = 0;
    rewinddir(dirp);
    read_files(dirp); // handle files
    closedir(dirp);
    break;
  }

  child_reaper(1);
  pipe_send();
}

int main(int argc, char *argv[]) {
  init(argc, argv, &prog_props);
  set_signals();

  path_handler(argv[0]); // fork subdirs and process files
  exit_log(EXIT_SUCCESS);
}
