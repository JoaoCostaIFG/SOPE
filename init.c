#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "include/init.h"
#include "include/logs.h"

void print_usage() {
  fprintf(stderr,
          "simpledu -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]\n");
  exit_log(1);
}

void init(int argc, char **argv, cmd_opt* cmd_opts) {
  /* clrlogs(); */
  set_logfile(getenv(LOG_ENV_NAME));

  write_create_log(argc, argv);

  // init cmd line options struct
  cmd_opts->all = 0;
  cmd_opts->bytes = 0;
  cmd_opts->block_size = 1024;
  cmd_opts->count_links = 1;
  cmd_opts->dereference = 0;
  cmd_opts->separate_dirs = 0;
  cmd_opts->max_depth = -1;
  cmd_opts->path[0] = '\0';

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
        cmd_opts->max_depth = atoi(optarg);
      else
        print_usage();
      break;
    case 'a':
      cmd_opts->all = 1;
      break;
    case 'b':
      cmd_opts->bytes = 1;
      break;
    case 'B':
      if (!optarg)
        print_usage();

      if (optarg[0] == '=')
        cmd_opts->block_size = atoi(optarg + 1);
      else
        cmd_opts->block_size = atoi(optarg);

      // error checking
      if (cmd_opts->block_size == 0) // atoi fails or 0
        print_usage();
      break;
    case 'l':
      cmd_opts->count_links = 1;
      break;
    case 'L':
      cmd_opts->dereference = 1;
      break;
    case 'S':
      cmd_opts->separate_dirs = 1;
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
      strcpy(cmd_opts->path, argv[optind++]);
  } else {
    fprintf(stderr, "No file/dir path given.\n");
    print_usage();
  }
}
