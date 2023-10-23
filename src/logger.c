#include "../include/logger.h"

static FILE *log_file;

void logger_init(const char *log_filename) {
  log_file = fopen(log_filename, "w");
  if (log_file == NULL) {
    ERROR_COMMENT("logger::logger_init", "File not found/not exists");
  }
}

void logger_close() { fclose(log_file); }

void ffflush() { fflush(log_file); }

void log_path(const int *path, int nnodes) {
  printf("log_path\n");
  return;
  char str[10000];
  int i;
  for (i = 0; i < nnodes; i++) {
    sprintf(str + strlen(str), "%d,", path[i]);
  }
  DEBUG_COMMENT("log_path", "%s,", str);
}

// char *getPath(const int *path, int nnodes) {
//   char *path_str = (char *)malloc(sizeof(char) * nnodes * 5);
//   sprintf(path_str, "[");
//   for (int i = 0; i < nnodes; i++) {
//     sprintf(path_str + strlen(path_str), "\t%d,", path[i]);
//   }
//   sprintf(path_str + strlen(path_str), "]");
//   return path_str;
// }

// void log_distancematrix(const double *distance_matrix, int nnodes) {
//   fprintf(log_file, "%lf;", distance_matrix[0]);
//   for (int i = 1; i < pow(nnodes, 2); i++) {
//     if (i % nnodes == 0) fprintf(log_file, "\n");
//     fprintf(log_file, "%lf;", distance_matrix[i]);
//   }
//   fprintf(log_file, "\n");
// }

void log_message(LogLevel level, const char *namefile_and_func,
                 const char *format, ...) {
  // Get the current time
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);
  char combined_str[200];

  // Get the log level string
  const char *level_str;
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
  case OUTPUT:
    level_str = "OUTPUT";
    break;
  case FATAL:
    level_str = "FATAL";
    break;
  default:
    level_str = "";
    break;
  }

  // Write the log message to the log file
  va_list args;
  sprintf(combined_str, "%s | %s", namefile_and_func, format);
  va_start(args, format);
  fprintf(log_file, "%04d-%02d-%02d %02d:%02d:%02d [%s]\t",
          tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
          tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, level_str);

  vfprintf(log_file, combined_str, args);
  fprintf(log_file, "\n");
  va_end(args);
  ffflush();
}
