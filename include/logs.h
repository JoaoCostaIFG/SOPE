/** @file logs.h */
#ifndef LOGS_H
#define LOGS_H

#define MAX_PATH_SIZE 256
/** log file name */
#define LOG_FILE "simpledu_log.txt"

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

/** @brief Write exit log message and exit with given code */
void exit_log(int exit_code);

/** @brief Sets the log file name. */
void set_logfile(char *new_logfile);

/** @brief Clears the log file. */
void clrlogs(void);

#endif // __ERR_UTILS_H__
