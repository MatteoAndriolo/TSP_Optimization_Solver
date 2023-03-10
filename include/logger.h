#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

// TODO implement system to check/remove messages and or check for levels 
// Define the log levels
typedef enum {
    ERROR,
    WARNING,
    CRITICAL,
    DEBUG,
    INFO,
} LogLevel;

// Initialize the logger
void logger_init(const char* log_filename);

// Log a message at the given log level
void log_message(LogLevel level,const char* namefile_and_func, const char* format, ...);
void ffflush();

// implement automatic flush

#endif




