/** @file utls.h */
#ifndef UTLS_H
#define UTLS_H

#define MAX32BITLEN 12
#define MAX64BITLEN 21
#define MAX128BITLEN 50

int set_env_int(char *name, int val, int overwrite);

int set_env_long(char *name, long val, int overwrite);

int set_env_longlong(char *name, long long val, int overwrite);

int get_env_int(char *name, int *val);

int get_env_long(char *name, long *val);

int get_env_longlong(char *name, long long *val);

int get_upstream_fd(int *upstream_fd);

#endif // UTLS_H
