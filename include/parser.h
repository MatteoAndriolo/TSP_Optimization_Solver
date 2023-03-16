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

/**
 * Prints a usage message to the given file stream and exits with the given status.
 * 
 * @param fp the file stream to print the usage message to
 * @param status the exit status
 */
void usage(FILE *fp, int status);

/**
 * Shows the available models and exits with the given status.
 * 
 * @param fp the file stream to print the models to
 * @param status the exit status
 */
void show_models(FILE *fp, int status);

/**
 * Prints the arguments of the given instance to the given file stream.
 * 
 * @param fp the file stream to print the arguments to
 * @param inst a pointer to the instance to print the arguments of
 */
void print_arguments(FILE *fp, const Instance* inst);

/**
 * Parses the command line arguments and stores the results in the given arguments struct.
 * 
 * @param argc the number of command line arguments
 * @param argv an array of command line argument strings
 * @param args a pointer to the arguments struct to store the results in
 */
void parse_command_line(int argc, char **argv, struct arguments *args);

/**
 * Reads the nodes file for the given instance, initializes the instance with the read data, and calculates the distance matrix.
 * 
 * @param inst a pointer to the instance to initialize
 */
void read_input(Instance *inst);


#endif