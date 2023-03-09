#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

// Define the log levels
typedef enum {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
} LogLevel;

// Initialize the logger
void logger_init(const char* log_filename);

// Log a message at the given log level
void log_message(LogLevel level,const char* namefile_and_func, const char* format, ...);

#endif




