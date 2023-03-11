#ifndef PARSLIB_H
#define PARSLIB_H

#include <argp.h>
#include <stdio.h>
#include "vrp.h"

// Parse arguments
struct arguments {
    Instance inst;
    int help;
    int help_models;
};

void usage(FILE *fp, int status);
void show_models(FILE *fp, int status);
void print_arguments(FILE *fp, const Instance* inst);
void parse_command_line(int argc, char  **argv, struct arguments *args);

// Read nodes file
void read_input(Instance *inst);

#endif