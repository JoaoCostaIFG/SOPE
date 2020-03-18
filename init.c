#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/init.h"
#include "include/logs.h"
#include "include/sigs.h"

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
  if (!str)
    return 0;

  size_t i = 0;
  if (str[0] == '-' || str[0] == '+') {
    if (strlen(str) <= 1)
      return 0;
    else
      ++i;
  }

  for (; i < strlen(str); ++i)
    if (!isdigit(str[i]))
      return 0;
  return 1;
}

void init(int argc, char **argv, cmd_opt *cmd_opts) {
  save_starttime();
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

        errno = 0;
        cmd_opts->max_depth = strtol(optarg, NULL, 10);
        if (errno == ERANGE)
          exit_err_log(INIT, "Invalid max depth argument. It is too large");
        else if (cmd_opts->block_size < 0)
          exit_err_log(INIT, "Invalid max depth argument. It must be >= 0");
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

      errno = 0;
      if (optarg[0] == '=' && is_str_num(optarg + 1))
        cmd_opts->block_size = strtol(optarg + 1, NULL, 10);
      else if (is_str_num(optarg))
        cmd_opts->block_size = strtol(optarg, NULL, 10);
      else
        print_usage();

      if (errno == ERANGE)
        exit_err_log(INIT, "Invalid block size argument. It is too large");
      else if (cmd_opts->block_size <= 0)
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

char *assemble_args(char *argv0, cmd_opt *cmd_opts) {
  /* args normais, B, depth, path, terminating NULL */
  char *args = (char *)malloc(sizeof(char) * MAX_PATH_SIZE);
  if (!args)
    exit_log(MALLOC_FAIL);

  strcpy(args, argv0);

  char tmp[40];
  sprintf(tmp, "-%s%s%s%s%s", cmd_opts->all ? "a" : "",
          cmd_opts->bytes ? "b" : "", cmd_opts->count_links ? "l" : "",
          cmd_opts->dereference ? "L" : "", cmd_opts->separate_dirs ? "S" : "");
  if (strcmp(tmp, "-")) {
    strcat(args, " ");
    strcat(args, tmp);
  }

  if (cmd_opts->block_size != 1024) {
    strcat(args, " ");
    sprintf(tmp, "-B=%ld", cmd_opts->block_size);
    strcat(args, tmp);
  }

  if (cmd_opts->max_depth >= 0) {
    strcat(args, " ");
    sprintf(tmp, "--max-depth=%ld", cmd_opts->max_depth);
    strcat(args, tmp);
  } else if (cmd_opts->max_depth == -2) {
    strcat(args, " --max_depth=0");
  }

  /* path */
  strcat(args, " ");
  strcat(args, cmd_opts->path);
  return args;
}

void init_child(char* argv0, char* new_path, cmd_opt *cmd_opts) {
  // cmd line args
  strcpy(cmd_opts->path, new_path);
  char *tmp;
  tmp = assemble_args(argv0, cmd_opts);
  LOG_CREATE(tmp);
  free(tmp);

  set_signals();

  if (cmd_opts->max_depth == 1)
    cmd_opts->max_depth = -2; // print only the dir
  else if (cmd_opts->max_depth == -2)
    cmd_opts->max_depth = 0; // don't print anything else
  else if (cmd_opts->max_depth > 0)
    --cmd_opts->max_depth; // lower one lvl if not -1 (infinite)
}
