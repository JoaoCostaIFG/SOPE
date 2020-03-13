#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "include/init.h"
#include "include/logs.h"

#define STAT_DFLT_SIZE 512

#define ARGS_MTR_SIZE 6

char **assemble_args(char *pname, cmd_opt *cmd_opts, char *file_name) {
  /* args normais, B, depth, path, terminating NULL */
  char **args = (char **)malloc(sizeof(char *) * ARGS_MTR_SIZE);
  if (!args)
    exit_log(-1);

  /* save program name (argv[0]) */
  if (!(args[0] = (char *)malloc(sizeof(char) * (strlen(pname) + 1))))
    exit_log(-1);
  strcpy(args[0], pname);

  /* args */
  int i = 1; // curr arg postion
  args[i] = malloc(sizeof(char) * 7);
  sprintf(args[i++], "-%s%s%s%s%s", cmd_opts->all ? "a" : "",
          cmd_opts->bytes ? "b" : "", cmd_opts->count_links ? "l" : "",
          cmd_opts->dereference ? "L" : "", cmd_opts->separate_dirs ? "S" : "");
  if (!strcmp(args[1], "-"))
    free(args[1]);

  if (cmd_opts->block_size != 1024) {
    if (!(args[i] = (char *)malloc(sizeof(char) * 16)))
      exit_log(-1);
    sprintf(args[i++], "-B=%d", cmd_opts->block_size);
  }

  if (cmd_opts->max_depth != -1) { // decrease max-depth
    if (!(args[i] = (char *)malloc(sizeof(char) * 24)))
      exit_log(-1);
    sprintf(args[i++], "--max-depth=%d", cmd_opts->max_depth - 1);
  }

  /* path */
  if (!(args[i] = (char *)malloc(
            sizeof(char) * (strlen(cmd_opts->path) + strlen(file_name) + 2))))
    exit_log(-1);
  sprintf(args[i++], "%s/%s", cmd_opts->path, file_name);

  args[i] = (char *)0;
  return args;
}

void handle_dir(cmd_opt *cmd_opts, char *pname) {
  // TODO dir first, then files
  DIR *dirp;
  if ((dirp = opendir(cmd_opts->path)) == NULL) {
    perror(cmd_opts->path);
    exit_log(2);
  }

  struct stat stat_buf;
  unsigned long size = 0;
  struct dirent *direntp;
  char path[MAX_PATH_SIZE];
  while ((direntp = readdir(dirp))) {
    if (!strcmp(direntp->d_name, ".") ||
        !strcmp(direntp->d_name, "..")) // skip "." && ".."
      continue;

    sprintf(path, "%s/%s", cmd_opts->path, direntp->d_name);
    if ((cmd_opts->dereference ? stat(path, &stat_buf)
                               : lstat(path, &stat_buf)) == -1)
      exit_log(2);

    size = cmd_opts->bytes
               ? stat_buf.st_size
               : stat_buf.st_blocks * STAT_DFLT_SIZE / cmd_opts->block_size;

    if (S_ISREG(stat_buf.st_mode) && cmd_opts->all)
      printf("%lu\t%s - %s\n", size, direntp->d_name, "regular");
    else if (S_ISLNK(stat_buf.st_mode) && cmd_opts->all)
      printf("%lu\t%s - %s\n", size, direntp->d_name, "link");
    else if (S_ISDIR(stat_buf.st_mode)) {
      pid_t pid = fork();
      switch (pid) {
      case -1:
        exit_log(-1);
        break;
      case 0: // child
        execv(pname, assemble_args(pname, cmd_opts, direntp->d_name));
        exit_log(3);
        break;
      default:
        break;
      }
    }
  }

  closedir(dirp);
}

int main(int argc, char *argv[]) {
  cmd_opt cmd_opts;
  init(argc, argv, &cmd_opts);

  if (cmd_opts.max_depth == 0) // max depth reached, quit
    exit_log(0);

  /* test if file was given, and handle it */
  int stat_ec;
  struct stat stat_buf;
  stat_ec = cmd_opts.dereference ? stat(cmd_opts.path, &stat_buf)
                                 : lstat(cmd_opts.path, &stat_buf);
  if (stat_ec == -1) // failed stat
    exit_log(2);
  else if (!S_ISDIR(stat_buf.st_mode)) {
    printf("%lu\t%s\n",
           cmd_opts.bytes
               ? stat_buf.st_size
               : stat_buf.st_blocks * STAT_DFLT_SIZE / cmd_opts.block_size,
           cmd_opts.path);
    exit_log(0);
  }

  handle_dir(&cmd_opts, argv[0]);

  exit_log(0);
}
