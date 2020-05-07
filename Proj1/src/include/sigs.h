/** @file sigs.h */
#ifndef SIGS_H
#define SIGS_H

#include <sys/types.h>

#include "parent_interface.h"

pid_t getGrpId(void);

void sigint_handler(int signum);

void set_child_sig(void);

void set_grandparent_sig(void);

void set_signals(void);

#endif // SIGS_H
