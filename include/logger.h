#ifndef LOGGER_H
#define LOGGER_H

#ifndef PRODUCTION
#define DEBUG_COMMENT(position,... ) log_message( DEBUG , position, __VA_ARGS__)
#define INFO_COMMENT(position, ... ) log_message( INFO , position, __VA_ARGS__)
#define WARNING_COMMENT(position, ... ) log_message( WARNING , position, __VA_ARGS__)
#define ERROR_COMMENT(position, ... ) log_message( ERROR , position, __VA_ARGS__)
#define CRITICAL_COMMENT(position, ... ) log_message( CRITICAL , position, __VA_ARGS__)
#define OUTPUT_COMMENT(position, ... ) log_message( OUTPUT , position, __VA_ARGS__)
#else
#define DEBUG_COMMENT(position, message) 
#define INFO_COMMENT(position, message)
#define WARNING_COMMENT(position, message)
#define ERROR_COMMENT(position, message)
#define CRITICAL_COMMENT(position, message)
#define OUTPUT_COMMENT(position, message) log_message( OUTPUT , position, message)
#endif

#include <stdio.h>

// TODO implement system to check/remove messages and or check for levels 
// Define the log levels
typedef enum {
    ERROR,
    WARNING,
    CRITICAL,
    DEBUG,
    INFO,
    OUTPUT,
} LogLevel;

// Initialize the logger
void logger_init(const char* log_filename);

// Log a message at the given log level
void log_message(LogLevel level,const char* namefile_and_func, const char* format, ...);
void ffflush();

#endif



