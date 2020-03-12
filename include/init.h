/** @file init.h */
#ifndef INIT_H
#define INIT_H

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
typedef struct cmd_opt cmd_opt;

void init(int argc, char **argv, cmd_opt* cmd_opts);

#endif // INIT_H
