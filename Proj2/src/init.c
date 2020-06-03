#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "include/init.h"
#include "include/logs.h"

#define IS_CLIENT 0
#define IS_SERVER 1

void
print_usage(char* argv0, int is_server)
{
  if (is_server)
    fprintf(
      stderr, "%s <-t nsecs> [-l nplaces] [-n nthreads] fifoname\n", argv0);
  else
    fprintf(stderr, "%s <-t nsecs> fifoname\n", argv0);

  exit(EXIT_FAILURE);
}

int
is_str_num(char* str)
{
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

void
init_prog_props(prog_prop* prog_props)
{
  // init cmd line options struct
  prog_props->nsecs    = EMPTY_FIELD;
  prog_props->nplaces  = EMPTY_FIELD;
  prog_props->nthreads = EMPTY_FIELD;
  strcpy(prog_props->fifoname, "");
}

void
init_u(int argc, char** argv, prog_prop* prog_props)
{
  init_prog_props(prog_props);

  int got_t = 0;
  int c;
  while ((c = getopt(argc, argv, "t:")) != -1) {
    switch (c) {
      case 't':
        if (!is_str_num(optarg))
          print_usage(argv[0], IS_CLIENT);

        got_t             = 1;
        prog_props->nsecs = strtol(optarg, NULL, 10);
        if (prog_props->nsecs <= 0)
          print_usage(argv[0], IS_CLIENT);
        break;
      case '?': // invalid option
        print_usage(argv[0], IS_CLIENT);
        break;
      default:
        fprintf(stderr, "getopt returned character code %#X\n", c);
        break;
    }
  }

  while (optind < argc)
    strcat(prog_props->fifoname, argv[optind++]);

  if (!strcmp(prog_props->fifoname, "") || !got_t)
    print_usage(argv[0], IS_CLIENT);
}

void
init_q(int argc, char** argv, prog_prop* prog_props)
{
  init_prog_props(prog_props);

  int got_t = 0;
  int c;
  while ((c = getopt(argc, argv, "l:n:t:")) != -1) {
    switch (c) {
      case 'l':
        if (!is_str_num(optarg))
          print_usage(argv[0], IS_SERVER);

        prog_props->nplaces = strtol(optarg, NULL, 10);
        if (prog_props->nplaces <= 0)
          print_usage(argv[0], IS_SERVER);
        break;
      case 'n':
        if (!is_str_num(optarg))
          print_usage(argv[0], IS_SERVER);

        prog_props->nthreads = strtol(optarg, NULL, 10);
        if (prog_props->nthreads <= 0)
          print_usage(argv[0], IS_SERVER);
        break;
      case 't':
        if (!is_str_num(optarg))
          print_usage(argv[0], IS_SERVER);

        got_t             = 1;
        prog_props->nsecs = strtol(optarg, NULL, 10);
        if (prog_props->nsecs <= 0)
          print_usage(argv[0], IS_SERVER);
        break;
      case '?': // invalid option
        print_usage(argv[0], IS_SERVER);
        break;
      default:
        fprintf(stderr, "getopt returned character code %#X\n", c);
        break;
    }
  }

  while (optind < argc)
    strcat(prog_props->fifoname, argv[optind++]);

  if (!strcmp(prog_props->fifoname, "") || !got_t)
    print_usage(argv[0], IS_SERVER);
}
