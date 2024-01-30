#ifndef PARSLIB_H
#define PARSLIB_H

#include <argp.h>
#include <stdbool.h>
#include <stdio.h>

#include "vrp.h"

// Parse arguments
typedef struct args {
  int num_instances;
  char model_type[256];
  bool integer_costs;
  int randomseed;
  long timelimit;
  char input_file[256];
  int nnodes;
  double *x, *y;
  char log_file[256];
  char grasp[20];
  int n_probabilities;
  double *grasp_probabilities;
  char str_probabilities[20];
  int toplot;
  int tabu_size;
  int tabu_tenure;
  double cplex_perchf;
  bool perfprof;
  char *paths_file;
} Args;

/**
 * Struct that contains the output of a run.
 */
typedef struct output {
  int iteration;
  double tlstart;
  int n_passagges;
  double *intermediate_tour_length;
  double tlfinal;
  double duration;
} Output;

char *sprintHeaderOutput(Output *output);

char *sprintOutput(Output *output);

void writeOutput(Output **output, FILE *fp, int num_instances);

static const char delimiter = '.';

/**
 * Reads the nodes file for the given instance, initializes the instance with
 *i the read data, and calculates the distance matrix.
 *
 * @param inst a pointer to the instance to initialize
 */
void read_input(Args *args);

/**
 * Prints the arguments of the given instance to the given file stream.
 *
 * @param fp the file stream to print the arguments to
 * @param inst a pointer to the instance to print the arguments of
 */
void print_arguments(const Args *args);

/**
 * Parses the command line arguments and stores the results in the given
 * arguments struct.
 *
 * @param argc the number of command line arguments
 * @param argv an array of command line argument strings
 * @param args a pointer to the arguments struct to store the results in
 */
void parse_command_line(int argc, char **argv, Args *args);

/***
 * Generate sequence of passagges from model name
 *
 * @param model_type the model name
 * @param passagges the sequence of passagges
 * @param n_passagges the number of passagges
 */
void parse_model_name(char *model_type, char ***passagges, int *n_passagges);

/**
 * Parse grasp probabilities
 *
 * @param grasp the grasp string
 * @param probabilities the probabilities
 * @param n_probabilities the number of probabilities
 */
void parse_grasp_probabilities(char *grasp, double *probabilities,
                               int *n_probabilities);

/**
 * Write staring path for each instance
 * dimentions! file must contain in the first line the number of dimensions
 *
 * @param infile the input filename for dimensions
 * @param outfile the output filename
 */

void writePermutationsToFile(const char *infile);

void readPermutations(const char *filename, int ***array, int *pn, int *ps);

void freePermutations(int n, int s, int ***array);
#endif
