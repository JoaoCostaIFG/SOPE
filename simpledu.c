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
#include "include/parent_interface.h"
#include "include/utls.h"

#define STAT_DFLT_SIZE 512
#define MAX_CHILDREN 500

#define READ 0
#define WRITE 1

#define SINGLE_REAP 3
#define REAP_ALL -1

struct child_elem {
  pid_t pid; // if > 0 pid o process, if -1 process dead
  char *path;
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

    if (close(prog_props.upstream_fd) == -1)
      exit_perror_log(PIPE_FAIL, "");
    write_sendpipe_log(my_size); // log pipe send
  } else {
    printf("%lu\t%s\n", lu_ceil((double)my_size / prog_props.block_size),
           prog_props.path);
    fflush(stdout);

    write_entry_log(my_size, prog_props.path);
  }
}

void rm_child(pid_t pid) {
  if (pid < 0)
    return;

  int read_len;
  char pipe_content[PIPE_BUF + 1];
  unsigned long size;
  char entry[MAX_PATH_SIZE];
  for (size_t i = 0; i < prog_props.child_num; ++i) {
    if (children[i].pid == pid) {
      children[i].pid = -1;

      // read pipe info
      if ((read_len = read(children[i].fd, pipe_content, PIPE_BUF)) == -1)
        exit_perror_log(PIPE_FAIL, "Reading from pipe failure");
      else if (read_len == 0)
        size = 0;
      else {
        if (sscanf(pipe_content, "%lu", &size) != 1)
          size = 0;
      }
      if (close(children[i].fd) == -1)
        exit_perror_log(PIPE_FAIL, "");

      /* logs */
      LOG_RECVPIPE(pipe_content);
      pathcpycat(entry, prog_props.path, children[i].path);
      write_entry_log(size, entry);
      free(children[i].path);

      /* write entry info */
      if (prog_props.max_depth != 0) {
        printf("%lu\t%s\n", lu_ceil((double)size / prog_props.block_size),
               entry);
        fflush(stdout);
      }

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
                                : lstat(path, &stat_buf)) < 0) {
      perror(path);
    }

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
                                : lstat(path, &stat_buf)) < 0) {
      perror(path);
    }

    if (S_ISDIR(stat_buf.st_mode)) {
      int fd[2]; // pipe to communicate size upstream
      pipe(fd);

      pid_t pid = fork(); // fork
      switch (pid) {
      case -1: // failed fork
        // TODO CHECK EAGAIN
        my_size += calc_size(&stat_buf);
        break;
      case 0: // child
        close(fd[READ]);
        /* close other pipes that we won't need */
        if (prog_props.upstream_fd != STDOUT_FILENO)
          close(prog_props.upstream_fd);
        for (size_t i = 0; i < prog_props.child_num; ++i)
          if (children[i].pid > 0)
            close(children[i].fd);

        init_child(argv, path, &prog_props);
        break;
      default: // parent
        close(fd[WRITE]);
        children[prog_props.child_num].fd = fd[READ];

        /* save child name */
        children[prog_props.child_num].path =
            (char *)malloc(sizeof(char) * (strlen(direntp->d_name) + 1));
        if (!children[prog_props.child_num].path)
          exit_log(MALLOC_FAIL);
        strcpy(children[prog_props.child_num].path, direntp->d_name);

        /* save pid and increase child count */
        children[prog_props.child_num++].pid = pid;
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

int main(int argc, char *argv[]) {
  init(argc, argv, &prog_props);

  /* fd memes */
  if (get_upstream_fd(&prog_props.upstream_fd))
    fputs("Failed to filter the current process' pipes", stderr);

  /* fork subdirs and process files */
  path_handler(argv);

  exit_log(EXIT_SUCCESS);
}
