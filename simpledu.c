#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "include/init.h"
#include "include/logs.h"

#define STAT_DFLT_SIZE 512

void handle_dir(cmd_opt* cmd_opts) {
  DIR *dirp;
  if ((dirp = opendir(cmd_opts->path)) == NULL) {
    perror(cmd_opts->path);
    exit_log(1);
  }
  chdir(cmd_opts->path);

  int stat_ec;
  struct stat stat_buf;
  unsigned long size = 0;
  struct dirent *direntp;
  while ((direntp = readdir(dirp)) != NULL) {
    stat_ec = cmd_opts->dereference ? stat(direntp->d_name, &stat_buf)
                                   : lstat(direntp->d_name, &stat_buf);
    if (stat_ec == -1) // failed stat
      exit_log(1);

    size = cmd_opts->bytes
               ? stat_buf.st_size
               : stat_buf.st_blocks * STAT_DFLT_SIZE / cmd_opts->block_size;

    if (S_ISREG(stat_buf.st_mode) && cmd_opts->all)
      printf("%lu %lu %s %s\n", direntp->d_ino, size, direntp->d_name,
             "regular");
    else if (S_ISLNK(stat_buf.st_mode) && cmd_opts->all)
      printf("%lu %lu %s %s\n", direntp->d_ino, size, direntp->d_name, "link");
  }

  closedir(dirp);
}

int main(int argc, char *argv[]) {
  cmd_opt cmd_opts;
  init(argc, argv, &cmd_opts);

  /* test if file was given, and handle it */
  int stat_ec;
  struct stat stat_buf;
  stat_ec = cmd_opts.dereference ? stat(cmd_opts.path, &stat_buf)
                                 : lstat(cmd_opts.path, &stat_buf);
  if (stat_ec == -1) // failed stat
    exit_log(1);
  else if (!S_ISDIR(stat_buf.st_mode)) {
    printf("%lu\t%s\n",
           cmd_opts.bytes
               ? stat_buf.st_size
               : stat_buf.st_blocks * STAT_DFLT_SIZE / cmd_opts.block_size,
           cmd_opts.path);
    exit_log(0);
  }

  handle_dir(&cmd_opts);

  exit_log(0);
}
