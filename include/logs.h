/** @file logs.h */
#ifndef LOGS_H
#define LOGS_H

/** max log file name size */
#define MAX_LOG_PATH_SIZE 256

/** log file directory */
#define LOG_DIR "/tmp/"

/** log file name */
#define LOG_FILE "simpledu_log.txt"

/** log file environment name */
#define LOG_ENV_NAME "LOG_FILENAME"

/** logs types */
#define CREATE_LOG "CREATE"
#define EXIT_LOG "EXIT"
#define RECVSIG_LOG "RECV_SIGNAL"
#define SENDSIG_LOG "SEND_SIGNAL"
#define RECVPIPE_LOG "RECV_PIPE"
#define SENDPIPE_LOG "SEND_PIPE"
#define ENTRY_LOG "ENTRY"

enum exit_codes {
  INIT = 1,
  FAILED_OPENDIR = 2,
  NON_EXISTING_ENTRY = 3,
  FORK_FAIL = 4,
  PIPE_FAIL = 5,
  EXEC_FAIL = 6,
  MALLOC_FAIL = 7,
  SIG_FAIL = 8,
  FILE_OPEN_ERROR = 9
};

/** write a log message */
void write_log(char *action, char *info);

/** log wrappers */
#define LOG_CREATE(info) write_log(CREATE_LOG, info)
#define LOG_EXIT(info) write_log(EXIT_LOG, info)
#define LOG_RECVSIG(info) write_log(RECVSIG_LOG, info)
#define LOG_SENDSIG(info) write_log(SENDSIG_LOG, info)
#define LOG_RECVPIPE(info) write_log(RECVPIPE_LOG, info)
#define LOG_SENDPIPE(info) write_log(SENDPIPE_LOG, info)
#define LOG_ENTRY(info) write_log(ENTRY_LOG, info)

/** @brief Format and write command line arguments to log */
void write_create_log(int argc, char **argv);

/** @brief Write exit log message and exit with given code */
void exit_log(int exit_code);

/** @brief Write stderr error message and call exit_log. */
void exit_err_log(int exit_code, char* msg);

/** @brief Write perror message and call exit_log. */
void exit_perror_log(int exit_code, char* msg);

/** @brief Sets the log file name. */
void set_logfile(char *new_logfile);

/** @brief Clears the log file. */
void clrlogs(void);

#endif // LOGS_H
