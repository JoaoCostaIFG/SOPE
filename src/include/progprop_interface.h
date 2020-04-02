/** @file progprop_interface.h */
#ifndef PROGPROP_INTERFACE_H
#define PROGPROP_INTERFACE_H

#include <stddef.h>

/** Max. file path size */
#define MAX_PATH_SIZE 4096

/** Default program options */
#define DFLT_ALL_OP 0
#define DFLT_BYTES_OP 0
#define DFLT_BLK_SIZE 1024
#define DFLT_CNT_LNK_OP 1
#define DFLT_DEREF_OP 0
#define DFLT_SEPDIRS_OP 0
#define DFLT_MAXDPTH_OP -1
#define DFLT_HAS_FAILED 0

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
  int has_failed;
  size_t child_num;
  int upstream_fd;
};
typedef struct prog_prop prog_prop;

#endif // PROGPROP_INTERFACE_H
