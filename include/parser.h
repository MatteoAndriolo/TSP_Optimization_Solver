#ifndef PARSLIB_H
#define PARSLIB_H

#include <argp.h>
#include <stdio.h>
#include "vrp.h"
#include "log.h"

// Parse arguments
struct arguments {
    Instance inst;
    int help;
};

void usage(FILE *fp, int status);
void print_arguments(FILE *fp, const Instance* inst);
void parse_command_line(int argc, char  **argv, struct arguments *args);

// Read nodes file
void read_input(Instance *inst);

#endif