#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "include/logs.h"

#define STAT_DFLT_SIZE 512

/** options */
struct cmd_opt {
  int all;
  int bytes;
  int block_size;
  int count_links;
  int dereference;
  int separate_dirs;
  int max_depth;
  char path[256];
};
static struct cmd_opt cmd_opt;

void print_usage() {
  fprintf(stderr,
          "simpledu -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]\n");
  exit_log(1);
}

void init(int argc, char **argv) {
  /* clrlogs(); */
  set_logfile(getenv(LOG_ENV_NAME));

  write_create_log(argc, argv);

  // init cmd line options struct
  cmd_opt.all = 0;
  cmd_opt.bytes = 0;
  cmd_opt.block_size = 1024;
  cmd_opt.count_links = 1;
  cmd_opt.dereference = 0;
  cmd_opt.separate_dirs = 0;
  cmd_opt.max_depth = -1;
  cmd_opt.path[0] = '\0';

  static struct option long_options[] = {
      {"all", no_argument, 0, 'a'},
      {"bytes", no_argument, 0, 'b'},
      {"block-size", required_argument, 0, 'B'},
      {"count-links", no_argument, 0, 'l'},
      {"dereference", no_argument, 0, 'L'},
      {"separate-dirs", no_argument, 0, 'S'},
      {"max-depth", required_argument, 0, 0},
      {0, 0, 0, 0}};

  int c, option_index = 0;
  while ((c = getopt_long(argc, argv, "abB:lLS", long_options,
                          &option_index)) != -1) {

    switch (c) {
    case 0:
      if (!strcmp(long_options[option_index].name, "max-depth") && optarg)
        cmd_opt.max_depth = atoi(optarg);
      else
        print_usage();
      break;
    case 'a':
      cmd_opt.all = 1;
      break;
    case 'b':
      cmd_opt.bytes = 1;
      break;
    case 'B':
      if (!optarg)
        print_usage();

      if (optarg[0] == '=')
        cmd_opt.block_size = atoi(optarg + 1);
      else
        cmd_opt.block_size = atoi(optarg);

      // error checking
      if (cmd_opt.block_size == 0) // atoi fails or 0
        exit_log(1);
      break;
    case 'l':
      cmd_opt.count_links = 1;
      break;
    case 'L':
      cmd_opt.dereference = 1;
      break;
    case 'S':
      cmd_opt.separate_dirs = 1;
      break;
    case '?': // invalid option
      print_usage();
      break;
    default:
      fprintf(stderr, "getopt returned character code %#X\n", c);
      break;
    }
  }

  // TODO check for multiple paths given?
  if (optind < argc) {
    while (optind < argc)
      strcpy(cmd_opt.path, argv[optind++]);
  }
}

int main(int argc, char *argv[]) {
  init(argc, argv);

  DIR *dirp;
  struct dirent *direntp;
  struct stat stat_buf;

  if ((dirp = opendir(cmd_opt.path)) == NULL) {
    perror(cmd_opt.path);
    exit_log(1);
  }

  chdir(cmd_opt.path);
  while ((direntp = readdir(dirp)) != NULL) {
    if (stat(direntp->d_name, &stat_buf) == -1) {
      perror("stroke no stat");
      exit_log(1);
    }
    
    unsigned long size = 0;
    if (cmd_opt.bytes)
      size = stat_buf.st_size;
    else
      size = stat_buf.st_blocks * STAT_DFLT_SIZE / cmd_opt.block_size;
    printf("%lu %lu %s %s\n", direntp->d_ino, size, direntp->d_name, "regular");
  }
  closedir(dirp);

  exit_log(0);
}
