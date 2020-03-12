#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/logs.h"

#define LOG_ENV_NAME "LOG_FILENAME"

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
  fprintf(stderr, "simpledu -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]\n");
  exit(1);
}

void init(int argc, char **argv) {
  set_logfile(getenv(LOG_ENV_NAME));

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
      printf("?? getopt returned character code 0%o ??\n", c);
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
  return 0;
}
