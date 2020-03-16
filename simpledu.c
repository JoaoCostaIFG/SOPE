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

#define READ 0
#define WRITE 1

static size_t child_num = 0;

struct child_elem {
  pid_t pid; // if > 0 pid o process, if -1 process dead
  int fd[2];
};
typedef struct child_elem child_elem;
static child_elem children[256];
static int parent_pipe[2];
static unsigned long my_size;

void pipe_send(cmd_opt *cmd_opts) {
  if (getpid() + 1 != get_pg_id()) {
    // write pipe
    FILE *fp;
    if ((fp = fdopen(parent_pipe[WRITE], "w")) == NULL)
      exit_perror_log(FILE_OPEN_ERROR, "Failure opening file");
    if (fprintf(fp, "%lu\n", my_size) < 0)
      exit_perror_log(PIPE_FAIL, "Writing to pipe failure");
    fclose(fp);
    write_sendpipe_log(my_size); // log pipe send
  } else {
    my_size += cmd_opts->bytes
                   ? DIR_NUM_BLK * DFLT_BLK_SIZE
                   : DIR_NUM_BLK * DFLT_BLK_SIZE / cmd_opts->block_size;
  }

  write_entry_log(my_size, cmd_opts->path);
  printf("%lu\t%s\n", my_size, cmd_opts->path);
  fflush(stdout);
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

void read_files(cmd_opt *cmd_opts) {
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

      write_entry_log(size, path);
      printf("%lu\t%s\n", size, path);
      fflush(stdout);
    }
  }

  closedir(dirp);
}

/*
 * char **assemble_args(char *pname, cmd_opt *cmd_opts) {
 *   [> args normais, B, depth, path, terminating NULL <]
 *   char **args = (char **)malloc(sizeof(char *) * ARGS_MTR_SIZE);
 *   if (!args)
 *     exit_log(-1);
 *
 *   [> save program name (argv[0]) <]
 *   if (!(args[0] = (char *)malloc(sizeof(char) * (strlen(pname) + 1))))
 *     exit_log(-1);
 *   strcpy(args[0], pname);
 *
 *   [> args <]
 *   int i = 1; // curr arg postion
 *   args[i] = malloc(sizeof(char) * 7);
 *   sprintf(args[i++], "-%s%s%s%s%s", cmd_opts->all ? "a" : "",
 *           cmd_opts->bytes ? "b" : "", cmd_opts->count_links ? "l" : "",
 *           cmd_opts->dereference ? "L" : "", cmd_opts->separate_dirs ? "S" : "");
 *   if (!strcmp(args[1], "-"))
 *     free(args[1]);
 *
 *   if (cmd_opts->block_size != 1024) {
 *     if (!(args[i] = (char *)malloc(sizeof(char) * 16)))
 *       exit_log(-1);
 *     sprintf(args[i++], "-B=%d", cmd_opts->block_size);
 *   }
 *
 *   if (cmd_opts->max_depth != -1) { // decrease max-depth
 *     if (!(args[i] = (char *)malloc(sizeof(char) * 24)))
 *       exit_log(-1);
 *     sprintf(args[i++], "--max-depth=%d", cmd_opts->max_depth - 1);
 *   }
 *
 *   [> path <]
 *   if (!(args[i] = (char *)malloc(sizeof(char) * (strlen(file_path) + 1))))
 *     exit_log(-1);
 *   strcpy(args[i++], file_path);
 *
 *   args[i] = (char *)0;
 *   return args;
 * }
 */

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
