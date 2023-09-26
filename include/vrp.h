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
#include "grasp.h"
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
  double max_time;  // overall time limit, in sec.s
  time_t tstart;    // starting time
  time_t tend;      // end time
  pthread_mutex_t mutex_path;

  // MODEL
  int integer_costs;

  // Data
  int nnodes;
  double *x;
  double *y;
  double *distance_matrix;

  // Paths
  int starting_node;
  int *path;
  int *best_path;
  double tour_length;
  double best_tl;  // best sol. available

  // GRASP parameters
  // int grasp_n_probabilities;
  // double *grasp_probabilities;
  GRASP_Framework *grasp;

  // global data
  char input_file[1000];  // input file
  char log_file[1000];    // output log file
} Instance;

void instance_initialize(Instance *inst, double max_time,
                         int integer_costs, int nnodes,
                         double *x,double *y, int grasp_n_probabilities,
                         double *grasp_probabilities, char *input_file,
                         char *log_file);
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

void instance_destroy(Instance *inst);

void swapPathPoints(Instance *inst, int i, int j);

int assertInst(Instance *inst);
/**
 * Generates a path for a set of nodes.
 *
 * @param path A pointer to an array that will store the path.
 * @param starting_node The index of the node to start the path at.
 * @param num_nodes The number of nodes in the graph.
 */
void instance_generate_path(Instance *inst);

void instance_generate_distance_matrix(Instance *inst);

#endif /* VRP_H_ */
