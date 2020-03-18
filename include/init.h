/** @file init.h */
#ifndef INIT_H
#define INIT_H

/** max file path size */
#define MAX_PATH_SIZE 4096

#define DFLT_BLK_SIZE 1024
#define DIR_NUM_BLK 4

/** options */
struct cmd_opt {
  int all;
  int bytes;
  long int block_size;
  int count_links;
  int dereference;
  int separate_dirs;
  long int max_depth;
  char path[MAX_PATH_SIZE + 1];
};
typedef struct cmd_opt cmd_opt;

/** @brief concatenate 2 strings separating them with '/' and clearing
 * duplicated '/' */
void pathcat(char *path, char *file_name);

/** @brief similar to pathcat but leaves stores result in res */
void pathcpycat(char *res, char *p1, char *p2);

void init(int argc, char **argv, cmd_opt *cmd_opts);

char *assemble_args(char *argv0, cmd_opt *cmd_opts);

void init_child(char* argv0, char* new_path, cmd_opt *cmd_opts);

#endif // INIT_H
