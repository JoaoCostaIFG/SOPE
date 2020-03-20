#include <dirent.h>
#include <errno.h>
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

#define SINGLE_REAP 3
#define REAP_ALL -1

struct child_elem {
  pid_t pid; // if > 0 pid o process, if -1 process dead
  int fd;
};
typedef struct child_elem child_elem;
static child_elem children[MAX_CHILDREN];

static prog_prop prog_props;
static unsigned long my_size;

unsigned long lu_ceil(double d) { return (unsigned long)ceil(d); }

unsigned long calc_size(struct stat *stat_buf) {
  return prog_props.bytes ? stat_buf->st_size
                          : stat_buf->st_blocks * STAT_DFLT_SIZE;
}

void pipe_send() {
  if (is_child()) {
    // write size to pipe
    char my_size_str[PIPE_BUF + 1];
    sprintf(my_size_str, "%lu\n", my_size);
    write(prog_props.upstream_fd, my_size_str, strlen(my_size_str));
    if (close(prog_props.upstream_fd) == -1) // TODO wtf esta linha crasha o prog
      perror("aqui o close rafado");

    write_sendpipe_log(my_size); // log pipe send

    printf("%lu\t%s\n", lu_ceil((double)my_size / prog_props.block_size),
           prog_props.path);
    fflush(stdout);
  } else {
    printf("%lu\t%s\n", lu_ceil((double)my_size / prog_props.block_size),
           prog_props.path);
    fflush(stdout);
  }

  write_entry_log(my_size, prog_props.path);
}

void rm_child(pid_t pid) {
  if (pid < 0)
    return;

  unsigned long size;
  for (size_t i = 0; i < prog_props.child_num; ++i) {
    if (children[i].pid == pid) {
      children[i].pid = -1;

      // read pipe info
      char pipe_content[PIPE_BUF + 1];
      int read_len;
      if ((read_len = read(children[i].fd, pipe_content, PIPE_BUF)) == -1)
        exit_perror_log(PIPE_FAIL, "reading from pipe failure");
      else if (read_len == 0)
        fprintf(stderr, "ler pipe  EOF - %s\n", prog_props.path);

      close(children[i].fd);

      /* write_recvpipe_log(my_size); // log pipe receive */
      write_log(RECVPIPE_LOG, pipe_content); // TODO

      sscanf(pipe_content, "%lu", &size);
      if (!prog_props.separate_dirs) // skip sub-dir size
        my_size += size;
      return;
    }
  }
}

void child_reaper(int reap_num) {
  if (!prog_props.child_num)
    return;

  if (reap_num == -1) {
    for (size_t i = 0; i < prog_props.child_num; ++i) {
      rm_child(wait(NULL));
    }
  } else {
    errno = 0;
    while (errno != ECHILD && reap_num > 0) {
      rm_child(waitpid(-1, NULL, WNOHANG));
      --reap_num;
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

    child_reaper(SINGLE_REAP); // reap children

    /* get formatted file path */
    pathcpycat(path, prog_props.path, direntp->d_name);
    if ((prog_props.dereference ? stat(path, &stat_buf)
                                : lstat(path, &stat_buf)) < 0)
      perror(path);

    // TODO FIFOS and other file types?
    /* if (S_ISREG(stat_buf.st_mode) || S_ISLNK(stat_buf.st_mode)) { */
    if (!S_ISDIR(stat_buf.st_mode)) {
      size = calc_size(&stat_buf);
      my_size += size;

      if (prog_props.all && prog_props.max_depth != 0) {
        // depth = 0 => last level
        printf("%lu\t%s\n", lu_ceil((double)size / prog_props.block_size),
               path);
        fflush(stdout);
      }
      // TODO acompanhar flags or sempre tamanho real ?
      write_entry_log(size, path);
    }
  }
}

void read_dirs(DIR *dirp, char **argv) {
  struct stat stat_buf;
  struct dirent *direntp;
  char path[MAX_PATH_SIZE];
  while ((direntp = readdir(dirp))) {
    /* skip "." && ".." */
    if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, ".."))
      continue;

    child_reaper(SINGLE_REAP); // reap children

    /* get formatted file path */
    pathcpycat(path, prog_props.path, direntp->d_name);
    if ((prog_props.dereference ? stat(path, &stat_buf)
                                : lstat(path, &stat_buf)) < 0)
      perror(path);

    if (S_ISDIR(stat_buf.st_mode)) {
      int fd[2];
      pipe(fd);           // pipe
      pid_t pid = fork(); // fork
      switch (pid) {
      case -1: // failed fork
        // TODO CHECK EAGAIN
        my_size += calc_size(&stat_buf);
        break;
      case 0: // child
        close(fd[READ]);
        /* close other pipes that we won't need */
        /*
         * if (prog_props.upstream_fd != STDOUT_FILENO)
         *   close(prog_props.upstream_fd);
         * for (size_t i = 0; i < prog_props.child_num; ++i)
         *   if (children[i].pid > 0)
         *     close(children[i].fd);
         */

        init_child(argv, path, &prog_props);
        break;
      default: // parent
        close(fd[WRITE]);
        children[prog_props.child_num].fd = fd[READ];
        ++prog_props.child_num;
        break;
      }
    }
  }
}

void path_handler(char **argv) {
  /* test if file was given, and handle it */
  struct stat stat_buf;
  if ((prog_props.dereference ? stat(prog_props.path, &stat_buf)
                              : lstat(prog_props.path, &stat_buf)) == -1)
    exit_perror_log(NON_EXISTING_ENTRY, prog_props.path);
  if (!S_ISDIR(stat_buf.st_mode)) {
    my_size = calc_size(&stat_buf);
    printf("%lu\t%s\n", lu_ceil((double)my_size / prog_props.block_size),
           prog_props.path);
    fflush(stdout);
    exit_log(EXIT_SUCCESS);
  }

  /* handle dires and their files/links */
  if ((prog_props.dereference ? stat(prog_props.path, &stat_buf)
                              : lstat(prog_props.path, &stat_buf)) < 0) {
    pipe_send();
    exit_perror_log(STAT_FAIL, prog_props.path);
  }
  my_size = calc_size(&stat_buf);

  DIR *dirp;
  if (!(dirp = opendir(prog_props.path))) {
    pipe_send();
    exit_perror_log(FAILED_OPENDIR, prog_props.path);
  }

  read_dirs(dirp, argv); // fork dirs
  rewinddir(dirp);
  read_files(dirp); // handle files
  closedir(dirp);

  child_reaper(REAP_ALL);
  pipe_send();
}

void get_parent_fd(void) {
  if (is_grandparent())
    return;

  char proc_path[50];
  sprintf(proc_path, "/proc/%d/fd/", getpid());
  char path[100];
  DIR *dirp = opendir(proc_path);
  struct dirent *direntp;
  struct stat stat_buf;
  while ((direntp = readdir(dirp))) {
    pathcpycat(path, proc_path, direntp->d_name);
    stat(path, &stat_buf);
    if (S_ISFIFO(stat_buf.st_mode)) { // is pipe?
      prog_props.upstream_fd = atoi(direntp->d_name);
      return;
    }
  }
}

int main(int argc, char *argv[]) {
  init(argc, argv, &prog_props);

  /* fd memes */
  get_parent_fd();

  /* fork subdirs and process files */
  path_handler(argv);

  exit_log(EXIT_SUCCESS);
}
