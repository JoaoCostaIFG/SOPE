/** @file utls.h */
#ifndef UTLS_H
#define UTLS_H

#include <stddef.h>
#include <sys/types.h>

#define MAX32BITLEN 12
#define MAX64BITLEN 21
#define MAX128BITLEN 50

#define SHM_NAME "shm"

int read_sigsafe(const int fd, void* buf, const size_t size);

int write_sigsafe(const int fd, const void* const buf, const size_t size);

int fifo_mk_safe(const char* const name, __mode_t mode);

int fifo_mk_open(const char* const name, __mode_t mode, const int o_flags);

#endif // UTLS_H
