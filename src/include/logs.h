/** @file logs.h */
#ifndef LOGS_H
#define LOGS_H

#include <sys/types.h>

#include "err_interface.h"

/** max log file name size */
#define MAX_LOG_PATH_SIZE 256

/** log file directory */
#define LOG_DIR "/tmp/"

/** log file name */
#define LOG_FILE "simpledu_log.txt"

/** write a log message */
void write_log(char *action, char *info);

/** logs types */
#define CREATE_LOG "CREATE"
#define EXIT_LOG "EXIT"
#define RECVSIG_LOG "RECV_SIGNAL"
#define SENDSIG_LOG "SEND_SIGNAL"
#define RECVPIPE_LOG "RECV_PIPE"
#define SENDPIPE_LOG "SEND_PIPE"
#define ENTRY_LOG "ENTRY"

/** log wrappers */
#define LOG_CREATE(info) write_log(CREATE_LOG, info)
#define LOG_EXIT(info) write_log(EXIT_LOG, info)
#define LOG_RECVSIG(info) write_log(RECVSIG_LOG, info)
#define LOG_SENDSIG(info) write_log(SENDSIG_LOG, info)
#define LOG_RECVPIPE(info) write_log(RECVPIPE_LOG, info)
#define LOG_SENDPIPE(info) write_log(SENDPIPE_LOG, info)
#define LOG_ENTRY(info) write_log(ENTRY_LOG, info)

/** @brief Format and write entry to log. */
void write_entry_log(unsigned long size, char *name);

/** @brief Format and write command line arguments to log. */
void write_create_log(char **argv);

/** @brief Format and write signal receiving event to log. */
void write_recvpipe_log(unsigned long info);

/** @brief Format and write signal sending event to log. */
void write_sendpipe_log(unsigned long info);

/** @brief Format and write signal receiving event to log. */
void write_recvsig_log(int signum);

/** @brief Format and write signal sending event to log. */
void write_sendsig_log(int signum, pid_t pid);

/** @brief Sets the log file name.
 *  @note If new_logfile is NULL, the name will be gotten from the env. var.
 * LOG_FILENAME
 */
void set_logfile(char *new_logfile);

/** @brief Clears the log file. */
void clrlogs(void);

void save_starttime(void);

void get_reftime(void);

#endif // LOGS_H
