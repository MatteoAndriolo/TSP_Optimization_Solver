#ifndef ERROR_CODES_H
#define ERROR_CODES_H
#include "../include/logger.h"
#include <stdio.h>

typedef enum {
  SUCCESS = 0,
  OK = 0,
  FAILURE = -1,
  ERROR_FILE_NOT_FOUND = -2,
  ERROR_INVALID_ARGUMENT = -3,
  ERROR_INVALID_PATH = -4,
  ERROR_NODES = -5,
  ERROR_TOUR_LENGTH = -6,
  ERROR_TIME_LIMIT = -99,
} ErrorCode;

/*
 * Run function that returns an error code and prints the error message
 * if the error code is not SUCCESS.
 * Manages errors
 *
 * @param x - function to run
 * @param ismain - if the function is main
 * @return error code
 */
#define _RUN(x, ismain)                                                        \
  do {                                                                         \
    ErrorCode err_code = (x);                                                  \
    if (err_code != SUCCESS) {                                                 \
      char err_name[40] = "";                                                  \
      switch (err_code) {                                                      \
      case FAILURE:                                                            \
        sprintf(err_name, "Failure");                                          \
        break;                                                                 \
      case ERROR_FILE_NOT_FOUND:                                               \
        sprintf(err_name, "File Not Found");                                   \
        break;                                                                 \
      case ERROR_INVALID_ARGUMENT:                                             \
        sprintf(err_name, "Invalid Argument");                                 \
        break;                                                                 \
      case ERROR_INVALID_PATH:                                                 \
        sprintf(err_name, "Invalid Path");                                     \
        break;                                                                 \
      case ERROR_NODES:                                                        \
        sprintf(err_name, "Node Error");                                       \
        break;                                                                 \
      case ERROR_TOUR_LENGTH:                                                  \
        sprintf(err_name, "Tour Length Error");                                \
        break;                                                                 \
      case ERROR_TIME_LIMIT:                                                   \
        sprintf(err_name, "Time Limit Reached");                               \
        break;                                                                 \
      default:                                                                 \
        sprintf(err_name, "Unknown Error");                                    \
        break;                                                                 \
      }                                                                        \
      printf("Error '%s' in %s:%d | %s \n", #x, __FILE__, __LINE__, err_name); \
      if (!ismain || err_code != ERROR_TIME_LIMIT)                             \
        return err_code;                                                       \
    }                                                                          \
  } while (false)

#define RUN(x) _RUN(x, false)
#define RUN_MAIN(x) _RUN(x, true)

//  #define _RUN(x, ismain)
//   do {
//     ErrorCode err_code = (x);
//     if (err_code != SUCCESS) {
//       ERROR_COMMENT("ERROR_RUN", "%s:%s -> %s", __FILE__, __LINE__, #x);
//       printf("Error running '%s' in %s:%d - ", #x, __FILE__, __LINE__);
//       switch (err_code) {
//       case FAILURE:
//         ERROR_COMMENT("errors.h:RUN", "Failure");
//         printf("Failure\n");
//         break;
//       case ERROR_FILE_NOT_FOUND:
//         ERROR_COMMENT("errors.h:RUN", "File Not Found");
//         printf("File Not Found\n");
//         break;
//       case ERROR_INVALID_ARGUMENT:
//         ERROR_COMMENT("errors.h:RUN", "Invalid Argument");
//         printf("Invalid Argument\n");
//         break;
//       case ERROR_INVALID_PATH:
//         ERROR_COMMENT("errors.h:RUN", "Invalid Path");
//         printf("Invalid Path\n");
//         break;
//       case ERROR_NODES:
//         ERROR_COMMENT("errors.h:RUN", "Node Error");
//         printf("Node Error\n");
//         break;
//       case ERROR_TOUR_LENGTH:
//         ERROR_COMMENT("errors.h:RUN", "Tour Length Error");
//         printf("Tour Length Error\n");
//         break;
//       case ERROR_TIME_LIMIT:
//         ERROR_COMMENT("errors.h:RUN", "Time Limit Reached");
//         printf("Time Limit Reached\n");
//         break;
//       default:
//         ERROR_COMMENT("errors.h:RUN", "Unknown Error");
//         printf("Unknown Error\n");
//         break;
//       }
//       if (!ismain || err_code != ERROR_TIME_LIMIT)
//         return err_code;
//     } else {
//       DEBUG_COMMENT("errors.h:RUN", "%s:%s -> %s", __FILE__, __LINE__, #x);
//     }
//   } while (0)
#endif
