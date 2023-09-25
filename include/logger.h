#ifndef LOGGER_H
#define LOGGER_H

#ifndef PRODUCTION
#define DEBUG_COMMENT(position, ...) log_message(DEBUG, position, __VA_ARGS__)
#define INFO_COMMENT(position, ...) log_message(INFO, position, __VA_ARGS__)
#define WARNING_COMMENT(position, ...) \
  log_message(WARNING, position, __VA_ARGS__)
#define ERROR_COMMENT(position, ...) log_message(ERROR, position, __VA_ARGS__)
#define CRITICAL_COMMENT(position, ...) \
  log_message(CRITICAL, position, __VA_ARGS__)
#define OUTPUT_COMMENT(position, ...) log_message(OUTPUT, position, __VA_ARGS__)
#define FATAL_COMMENT(position, ...)         \
  log_message(FATAL, position, __VA_ARGS__); \
  exit(1);
#else
#define DEBUG_COMMENT(position, ...)
#define INFO_COMMENT(position, ...) log_message(INFO, position, __VA_ARGS__)
#define WARNING_COMMENT(position, ...)
#define ERROR_COMMENT(position, ...) log_message(ERROR, position, __VA_ARGS__)
#define CRITICAL_COMMENT(position, ...)
#define OUTPUT_COMMENT(position, ...) log_message(OUTPUT, position, __VA_ARGS__)
#define FATAL_COMMENT(position, ...)         \
  log_message(FATAL, position, __VA_ARGS__); \
  exit(1);
#endif

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef enum {
  ERROR,
  WARNING,
  CRITICAL,
  DEBUG,
  INFO,
  OUTPUT,
  FATAL,
} LogLevel;

/**
 * Initializes the logger by creating a log file with the given name.
 *
 * @param log_filename the name of the log file to be created
 */
void logger_init(const char *log_filename);

/**
 * Logs a message at the given log level, including the name of the file and
 * function where the message is logged.
 *
 * @param level the log level of the message
 * @param namefile_and_func a string containing the name of the file and
 * function where the message is logged
 * @param format a printf-style format string for the log message
 * @param ... variable arguments corresponding to the format string
 */
void log_message(LogLevel level, const char *namefile_and_func,
                 const char *format, ...);

/**
 * Flushes the output buffer of the log file.
 */
void ffflush();

/**
 * Logs the given path.
 *
 * @param path the path to be logged
 * @param nnodes the number of nodes
 */
void log_path(const int *path, int nnodes);

/**
 * Return string the given path.
 *
 * @param path the path to be logged
 * @param nnodes the number of nodes
 */
char *getPath(const int *path, int nnodes);

/**
 *
 */
void log_distancematrix(const double *distance_matrix, int nnodes);

/**
 * Closes the log file.
 */
void logger_close();

#endif
