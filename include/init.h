/** @file init.h */
#ifndef INIT_H
#define INIT_H

/** max file path size */
#define MAX_PATH_SIZE 4096

/** options */
struct cmd_opt {
  int all;
  int bytes;
  int block_size;
  int count_links;
  int dereference;
  int separate_dirs;
  int max_depth;
  char path[MAX_PATH_SIZE + 1];
};
typedef struct cmd_opt cmd_opt;

void init(int argc, char **argv, cmd_opt *cmd_opts);

#endif // INIT_H
