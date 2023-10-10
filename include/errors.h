// error_codes.h
#ifndef ERROR_CODES_H
#define ERROR_CODES_H
#include "../include/logger.h"
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
      printf("Error running '%s' in %s:%d - ", #x, __FILE__, __LINE__);        \
      switch (err_code) {                                                      \
      case FAILURE:                                                            \
        printf("Failure\n");                                                   \
        break;                                                                 \
      case ERROR_FILE_NOT_FOUND:                                               \
        printf("File Not Found\n");                                            \
        break;                                                                 \
      case ERROR_INVALID_ARGUMENT:                                             \
        printf("Invalid Argument\n");                                          \
        break;                                                                 \
      case ERROR_INVALID_PATH:                                                 \
        printf("Invalid Path\n");                                              \
        break;                                                                 \
      case ERROR_NODES:                                                        \
        printf("Node Error\n");                                                \
        break;                                                                 \
      case ERROR_TOUR_LENGTH:                                                  \
        printf("Tour Length Error\n");                                         \
        break;                                                                 \
      case ERROR_TIME_LIMIT:                                                   \
        printf("Time Limit Reached\n");                                        \
        break;                                                                 \
      default:                                                                 \
        printf("Unknown Error\n");                                             \
        break;                                                                 \
      }                                                                        \
      if (!ismain || err_code != ERROR_TIME_LIMIT)                             \
        return err_code;                                                       \
      else {                                                                   \
        printf("Success running '%s' in %s:%d\n", #x, __FILE__, __LINE__);     \
        return SUCCESS;                                                        \
      }                                                                        \
    }                                                                          \
  } while (0)

//} else {
//    printf("Success running '%s' in %s:%d\n", #x, __FILE__, __LINE__);
//}

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
