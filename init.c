#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/init.h"
#include "include/logs.h"

void print_usage() {
  fprintf(stderr,
          "simpledu -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]\n");
  exit_log(INIT);
}

void pathcat(char *p1, char *p2) {
  size_t path_len = strlen(p1);
  size_t i;

  /* clear duplicated '/' from first path */
  for (i = 0; i < path_len; ++i) {
    if (p1[i] != '/' || p1[i + 1] != '/')
      continue;

    --i;
    --path_len;
    for (size_t j = i + 1; j < path_len - 1; ++j)
      p1[j] = p1[j + 1];
    --path_len;
  }

  /* ensure paths are separated by a '/' */
  if (i > 0 && p1[i - 1] != '/')
    p1[i++] = '/';

  /* concatenate both paths removing duplicate '/' from the second one */
  for (size_t j = 0; i < MAX_PATH_SIZE && j < strlen(p2); ++j)
    if (!(p1[i - 1] == '/' && p2[j] == '/') || i == 0)
      p1[i++] = p2[j];
  p1[i] = '\0'; // terminate string
}

void pathcpycat(char *res, char *p1, char *p2) {
  strcpy(res, p1);
  pathcat(res, p2);
}

int is_str_num(char *str) {
  for (size_t i = 0; i < strlen(str); ++i)
    if (!isdigit(str[i]))
      return 0;
  return 1;
}

void init(int argc, char **argv, cmd_opt *cmd_opts) {
  clrlogs();
  set_logfile(getenv(LOG_ENV_NAME));

  write_create_log(argc, argv);

  // init cmd line options struct
  cmd_opts->all = 0;
  cmd_opts->bytes = 0;
  cmd_opts->block_size = DFLT_BLK_SIZE;
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
      if (!strcmp(long_options[option_index].name, "max-depth") && optarg) {
        if (!is_str_num(optarg))
          print_usage();
        else if ((cmd_opts->max_depth = atoi(optarg)) < 0)
          exit_err_log(INIT, "Invalid maximum depth. It should be >= 0");
      } else
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
      if (optarg[0] == '=' && is_str_num(optarg + 1))
        cmd_opts->block_size = atoi(optarg + 1);
      else if (is_str_num(optarg))
        cmd_opts->block_size = atoi(optarg);
      else
        print_usage();
      if (cmd_opts->block_size == 0)
        exit_err_log(INIT, "Invalid block size argument. It must be > 0");
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
      pathcat(cmd_opts->path, argv[optind++]);
  } else {
    print_usage();
  }
}
