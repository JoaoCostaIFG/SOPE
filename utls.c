#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "include/logs.h"
#include "include/sigs.h"
#include "include/utls.h"

#define FILTER_ENV "SIMPLEDU_FILTER"

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

int set_fd_filter(void) {
  int fd_r;
  int fd_info_len;
  char fd_info[51];
  int filter_len;
  char *filter = NULL;

  /* get path to the current process' proc dir */
  char path[500];
  char proc_path[50];
  sprintf(proc_path, "/proc/%d/fd/", getpid());

  struct stat stat_buf;
  struct dirent *direntp;
  DIR *dirp;
  if (!(dirp = opendir(proc_path))) {
    exit_perror_log(FAILED_OPENDIR, proc_path);
  }

  while ((direntp = readdir(dirp))) {
    if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, ".."))
      continue;

    /* evaluate type */
    snprintf(path, 498, "%s/%s", proc_path, direntp->d_name);
    if (stat(path, &stat_buf))
      perror(path);

    if (S_ISFIFO(stat_buf.st_mode)) { // is pipe?
      fd_r = atoi(direntp->d_name);   // get file descriptor
      /* we don't want invalid file descriptors or the default streams */
      if (fd_r < 0 || fd_r == STDIN_FILENO || fd_r == STDOUT_FILENO ||
          fd_r == STDERR_FILENO)
        continue;

      if ((fd_info_len = readlink(path, fd_info, 50)) == -1) {
        // TODO Might need to add this to filter 2
        perror("Broken file descriptor link.");
        continue;
      }
      fd_info[fd_info_len] = '\0'; // readlink doesn't null-terminate strings

      /* alloc memory and assemble filter */
      if (filter == NULL) { // first item
        filter = (char *)malloc(sizeof(char) * (strlen(fd_info) + 2));
        strcpy(filter, fd_info);
        filter[fd_info_len] = '\n';
        filter[fd_info_len + 1] = '\0';
      } else {
        filter_len = strlen(filter) + strlen(fd_info);
        filter = realloc(filter, sizeof(char) * (filter_len + 1));

        strcat(filter, fd_info);
        filter[filter_len] = '\n';
        filter[filter_len + 1] = '\0';
      }
    }
  }

  if (filter == NULL) // nothing to filter
    filter = "";

  return setenv(FILTER_ENV, filter, 1);
}

int get_filtered_fd(int *upstream_fd) {
  /* getting the pipes to filter out */
  char *filter;
  if ((filter = getenv(FILTER_ENV)) == NULL) {
    /* weird failure */
    *upstream_fd = STDOUT_FILENO;
    return 1;
  }

  int fd_info_len;
  char fd_info[51];
  int fd_r;
  int candidate_num = 0;

  /* get path to the current process' proc dir */
  char path[500];
  char proc_path[50];
  sprintf(proc_path, "/proc/%d/fd/", getpid());

  struct stat stat_buf;
  struct dirent *direntp;
  DIR *dirp;
  if (!(dirp = opendir(proc_path))) {
    exit_perror_log(FAILED_OPENDIR, proc_path);
  }

  while ((direntp = readdir(dirp))) {
    if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, ".."))
      continue;

    /* evaluate type */
    snprintf(path, 498, "%s/%s", proc_path, direntp->d_name);
    if (stat(path, &stat_buf))
      perror(path);

    if (S_ISFIFO(stat_buf.st_mode)) { // is pipe?
      fd_r = atoi(direntp->d_name);   // get file descriptor
      /* we don't want invalid file descriptors or the default streams */
      if (fd_r < 0 || fd_r == STDIN_FILENO || fd_r == STDOUT_FILENO ||
          fd_r == STDERR_FILENO)
        continue;

      if ((fd_info_len = readlink(path, fd_info, 50)) == -1) {
        // TODO Might need to add this to filter 2
        perror("Broken file descriptor link.");
        continue;
      }
      fd_info[fd_info_len] = '\0'; // readlink doesn't null-terminate strings

      if (strstr(filter, fd_info) == NULL) {
        ++candidate_num;
        *upstream_fd = fd_r;
      }
    }
  }

  if (candidate_num != 1)
    return 1;

  return 0;
}

int get_upstream_fd(int *upstream_fd) {
  if (is_grandparent()) {
    *upstream_fd = STDOUT_FILENO;
    return set_fd_filter();
  } else {
    return get_filtered_fd(upstream_fd);
  }
}
