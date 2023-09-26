// error_codes.h
#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#define SUCCESS 0
#define FAILURE -1
#define ERROR_FILE_NOT_FOUND -2
#define ERROR_INVALID_ARGUMENT -3
#define ERROR_INVALID_PATH -4
#define ERROR_NODES -5
#define ERROR_TOUR_LENGTH -5

int check_error(int error_code);

#endif
