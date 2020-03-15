#ifndef SIGS_H
#define SIGS_H

#include <unistd.h>

void sigint_handler(int signum);

void set_child_sig(void);

void set_grandparent_sig(void);

void set_pg_id(pid_t pid);

pid_t get_pg_id(void);

#endif // SIGS_H
