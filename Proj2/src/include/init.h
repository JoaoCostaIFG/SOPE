/** @file init.h */
#ifndef INIT_H
#define INIT_H

#include <stddef.h>

#include "progprop_interface.h"

void init_u(int argc, char **argv, prog_prop *prog_props);

void init_q(int argc, char **argv, prog_prop *prog_props);

#endif // INIT_H
