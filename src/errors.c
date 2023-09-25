#include <stdio.h>
#include "../include/errors.h"

int check_error(int error_code){
    printf("Error code: %d\n", error_code);
    return SUCCESS;
}
