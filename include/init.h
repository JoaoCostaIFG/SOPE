/** @file init.h */
#ifndef INIT_H
#define INIT_H

#include <stdlib.h>

/** max file path size */
#define MAX_PATH_SIZE 4096

#define DFLT_BLK_SIZE 1024
#define DIR_NUM_BLK 4

/** options */
struct prog_prop {
  int all;
  int bytes;
  long int block_size;
  int count_links;
  int dereference;
  int separate_dirs;
  long int max_depth;
  char path[MAX_PATH_SIZE + 1];
  size_t child_num;
  int parent_pipe[2];
};
typedef struct prog_prop prog_prop;

/** @brief concatenate 2 strings separating them with '/' and clearing
 * duplicated '/' */
void pathcat(char *path, char *file_name);

/** @brief similar to pathcat but leaves stores result in res */
void pathcpycat(char *res, char *p1, char *p2);

void init(int argc, char **argv, prog_prop *prog_props);

int assemble_args(char **argv, prog_prop *prog_props);

void init_child(char **argv, char *new_path, prog_prop *prog_props);

#endif // INIT_H
