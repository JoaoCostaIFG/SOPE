/** @file err_interface.h */
#ifndef ERR_INTERFACE_H
#define ERR_INTERFACE_H

enum exit_codes {
  INIT = 1,
  FAILED_OPENDIR = 2,
  NON_EXISTING_ENTRY = 3,
  FORK_FAIL = 4,
  PIPE_FAIL = 5,
  EXEC_FAIL = 6,
  MALLOC_FAIL = 7,
  SIG_FAIL = 8,
  STAT_FAIL = 9,
  FILE_OPEN_ERROR = 10,
  TIME_ERROR = 11,
  ENV_ERROR = 12
};

/** @brief Write exit log message and exit with given code. */
void exit_log(int exit_code);

/** @brief Write stderr error message and call exit_log. */
void exit_err_log(int exit_code, char *msg);

/** @brief Write perror message and call exit_log. */
void exit_perror_log(int exit_code, char *msg);

#endif // ERR_INTERFACE_H
