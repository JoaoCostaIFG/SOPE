/** @file init.h */
#ifndef INIT_H
#define INIT_H

#include <stdlib.h>

#include "progprop_interface.h"

/** @brief concatenate 2 strings separating them with '/' and clearing
 * duplicated '/' */
void pathcat(char *path, char *file_name);

/** @brief similar to pathcat but leaves stores result in res */
void pathcpycat(char *res, char *p1, char *p2);

void init(int argc, char **argv, prog_prop *prog_props);

void assemble_args(char **argv, prog_prop *prog_props, char *new_path);

void init_child(char **argv, char *new_path, prog_prop *prog_props);

#endif // INIT_H
