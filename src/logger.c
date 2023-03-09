#include "logger.h"
#include <stdarg.h>
#include <time.h>
#include <string.h>

static FILE* log_file;

void logger_init(const char* log_filename) {
    log_file = fopen(log_filename, "w");
    if (log_file == NULL) {
       log_message(ERROR, "logger::logger_init","File not found/not exists");
    }
}

void log_message(LogLevel level, const char* namefile_and_func, const char* format, ...) {
    // Get the current time
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char combined_str[200];

    // Get the log level string
    const char* level_str;
    switch (level) {
        case DEBUG:
            level_str = "DEBUG";
            break;
        case INFO:
            level_str = "INFO";
            break;
        case WARNING:
            level_str = "WARNING";
            break;
        case ERROR:
            level_str = "ERROR";
            break;
        case CRITICAL:
            level_str = "CRITICAL";
            break;
        default:
            level_str = "";
            break;
    }

    // Write the log message to the log file
    va_list args;
    sprintf(combined_str, "%s %s", namefile_and_func , format);
    va_start(args, format);
    fprintf(log_file, "%04d-%02d-%02d %02d:%02d:%02d [%s] ", 
            tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday, 
            tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, level_str);
    
    vfprintf(log_file, combined_str, args);
    fprintf(log_file, "\n");
    va_end(args);
}
