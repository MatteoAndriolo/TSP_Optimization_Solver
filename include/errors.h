// error_codes.h
#ifndef ERROR_CODES_H
#define ERROR_CODES_H

typedef enum{
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



#define _RUN(x, ismain) \
    do { \
        ErrorCode err_code = (x); \
        if (err_code != SUCCESS) { \
            printf("Error running '%s' in %s:%d - ", #x, __FILE__, __LINE__); \
            switch (err_code) { \
                case FAILURE: printf("Failure\n"); break; \
                case ERROR_FILE_NOT_FOUND: printf("File Not Found\n"); break; \
                case ERROR_INVALID_ARGUMENT: printf("Invalid Argument\n"); break; \
                case ERROR_INVALID_PATH: printf("Invalid Path\n"); break; \
                case ERROR_NODES: printf("Node Error\n"); break; \
                case ERROR_TOUR_LENGTH: printf("Tour Length Error\n"); break; \
                case ERROR_TIME_LIMIT: printf("Time Limit Reached\n"); break; \
                default: printf("Unknown Error\n"); break; \
            } \
            if(!ismain || err_code!=ERROR_TIME_LIMIT) \
                return err_code; \
        } \
    } while (0)

//} else {
//    printf("Success running '%s' in %s:%d\n", #x, __FILE__, __LINE__);
//}

#define RUN(x) _RUN(x, false)
#define RUN_MAIN(x) _RUN(x, true)

#endif
