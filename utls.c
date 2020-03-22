#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/utls.h"

int set_env_int(char *name, int val, int overwrite) {
  char val_str[MAX128BITLEN];
  sprintf(val_str, "%d", val);

  return setenv(name, val_str, overwrite);
}

int set_env_long(char *name, long val, int overwrite) {
  char val_str[MAX128BITLEN];
  sprintf(val_str, "%ld", val);

  return setenv(name, val_str, overwrite);
}

int set_env_longlong(char *name, long long val, int overwrite) {
  char val_str[MAX128BITLEN];
  sprintf(val_str, "%lld", val);

  return setenv(name, val_str, overwrite);
}

int get_env_int(char *name, int *val) {
  char *tmp;
  if ((tmp = getenv(name)) == NULL)
    return 1;

  *val = (int)strtol(tmp, NULL, 10);
  return 0;
}

int get_env_long(char *name, long *val) {
  char *tmp;
  if ((tmp = getenv(name)) == NULL)
    return 1;

  *val = strtol(tmp, NULL, 10);
  return 0;
}

int get_env_longlong(char *name, long long *val) {
  char *tmp;
  if ((tmp = getenv(name)) == NULL)
    return 1;

  *val = strtoll(tmp, NULL, 10);
  return 0;
}
