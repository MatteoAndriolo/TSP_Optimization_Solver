#ifndef LOG_H
#define LOG_H
/*VERBOSITY
1) only the output
2) essencial passes
3) passes and some information
4) verbose almost all 
5) every think the log as well 
*/

void print_info(const char *msg);

void print_debug(const char *msg);

void print_error(const char *msg);

#endif