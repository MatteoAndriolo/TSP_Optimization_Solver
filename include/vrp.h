#ifndef VRP_H_
#define VRP_H_

#include <getopt.h>
#include <math.h>
// #include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <cplex.h>
#include <pthread.h>

#include "errors.h"
#include "logger.h"

// hard-wired parameters
#define XSMALL \
  1e-5  // 1e-4*	// tolerance used to decide ingerality of 0-1 var.s
#define EPSILON 1e-9  // 1e-9		// very small numerical tolerance
#define TICKS_PER_SECOND \
  1000.0  // cplex's ticks on Intel Core i7 quadcore @2.3GHZ
#define INFTY 1e+30

typedef struct {
  // execution parameters
  int randomseed;
  double timelimit;       // overall time limit, in sec.s
  char input_file[1000];  // input file
  char log_file[1000];    // output log file

  // MODEL
  int model_type;
  int integer_costs;

  // Data
  int nnodes;
  double *x;
  double *y;
  double *distance_matrix;

  // Paths
  int node_start;
  double tour_length;
  int *path;
  double zbest;  // best sol. available
  int *path_best;
  pthread_mutex_t mutex_path;

  // GRASP parameters
  int grasp_n_probabilities;
  double *grasp_probabilities;

  // global data
  double tstart;  // starting time
  double tend;    // end time

} Instance;

void initializeInstance(Instance *inst, int randomseed, double timelimit,
                        const char *input_file, const char *log_file,
                        int model_type, int integer_costs, int nnodes,
                        double *x, double *y, double *distance_matrix,
                        int node_start, double tour_length, int *path,
                        double zbest, int *path_best, int grasp_n_probabilities,
                        double *grasp_probabilities, double tstart,
                        double tend);

/*
 * Get distance between nodes in position x and y of path
 */
double getDistancePos(Instance *inst, int x, int y);

/*
 * Get distance between nodes x and y
 */
double getDistanceNodes(Instance *inst, int x, int y);

double calculateTourLength(Instance *inst);
void setTourLenght(Instance *inst, double newLength);
void addToTourLenght(Instance *inst, double toAdd);

void destroyInstance(Instance *inst);

void swapPathPoints(Instance *inst, int i, int j);

int assertInst(Instance *inst);
#endif /* VRP_H_ */
