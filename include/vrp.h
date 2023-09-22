#ifndef VRP_H_
#define VRP_H_

#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <cplex.h>
#include <pthread.h>

// hard-wired parameters
#define XSMALL \
  1e-5  // 1e-4*	// tolerance used to decide ingerality of 0-1 var.s
#define EPSILON 1e-9  // 1e-9		// very small numerical tolerance
#define TICKS_PER_SECOND \
  1000.0  // cplex's ticks on Intel Core i7 quadcore @2.3GHZ
#define INFTY 1e+30

typedef struct {
  // input data
  int nnodes;
  double *x;
  double *y;

  // MODEL
  int model_type;
  int integer_costs;

  double zbest;  // best sol. available
  double tour_lenght;
  int *path_best;

  int node_start;

  // GRASP parameters
  int grasp_n_probabilities;
  double *grasp_probabilities;

  // execution parameters
  int randomseed;
  double timelimit;       // overall time limit, in sec.s
  char input_file[1000];  // input file
  char log_file[1000];    // output log file

  // global data
  double tstart;  // starting time

} Instance;

void destroyInstance(Instance *inst);

// inline
inline int imax(int i1, int i2) { return (i1 > i2) ? i1 : i2; }
inline double dmin(double d1, double d2) { return (d1 < d2) ? d1 : d2; }
inline double dmax(double d1, double d2) { return (d1 > d2) ? d1 : d2; }
#endif /* VRP_H_ */
