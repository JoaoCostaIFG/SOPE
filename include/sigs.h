#ifndef SIGS_H
#define SIGS_H

#include <unistd.h>

pid_t getGrpId(void);

void set_grandparent(void);

int is_child(void);

int is_grandparent(void);

void sigint_handler(int signum);

void set_child_sig(void);

void set_grandparent_sig(void);

void set_signals(void);

#endif // SIGS_H
