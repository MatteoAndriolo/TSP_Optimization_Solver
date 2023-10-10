#ifndef VRP_H_
#define VRP_H_

#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <cplex.h>

#include "../tmpcplex/cplex.h"
#include "errors.h"
#include "grasp.h"
#include "logger.h"

// hard-wired parameters
#define XSMALL                                                                 \
  1e-5               // 1e-4*	// tolerance used to decide ingerality of 0-1
                     // var.s
#define EPSILON 1e-9 // 1e-9		// very small numerical tolerance
#define TICKS_PER_SECOND                                                       \
  1000.0 // cplex's ticks on Intel Core i7 quadcore @2.3GHZ
#define INFTY 1e+30

typedef enum {
  SOLVER_BASE = 0,
  SOLVER_BENDER = 1,
  SOLVER_PATCHING_HEURISTIC = 2,
  SOLVER_BRANCH_AND_CUT = 3,
  SOLVER_POSTINGHEU_UCUTFRACT = 4,
  SOLVER_MH_HARDFIX = 5,
  SOLVER_MH_LOCBRANCH = 6,
} TSPSolvers;

typedef struct {
  // execution parameters
  double max_time; // overall time limit, in sec.s
  time_t tstart;   // starting time
  time_t tend;     // end time
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
  double best_tourlength; // best sol. available

  // GRASP parameters
  GRASP_Framework *grasp;

  // TSP MODEL
  int percentageHF;
  TSPSolvers solver;
  CPXENVptr env;
  CPXLPptr lp;
  double *edgeList;
  int ncols;

  // global data
  char input_file[1000]; // input file
  char log_file[1000];   // output log file
} Instance;

void INSTANCE_initialize(Instance *inst, double max_time, int integer_costs,
                         int nnodes, double *x, double *y,
                         int grasp_n_probabilities, double *grasp_probabilities,
                         char *input_file, char *log_file);

void INSTANCE_free(Instance *inst);

void INSTANCE_generatePath(Instance *inst);

void INSTANCE_generateDistanceMatrix(Instance *inst);

// ----------------------------------------------------------------------------
// Distance related utils
// ----------------------------------------------------------------------------
double INSTANCE_getDistancePos(Instance *inst, int x, int y);

double INSTANCE_getDistanceNodes(Instance *inst, int x, int y);

double INSTANCE_calculateTourLength(Instance *inst);

void INSTANCE_setTourLenght(Instance *inst, double newLength);

void INSTANCE_addToTourLenght(Instance *inst, double toAdd);

void INSTANCE_saveBestPath(Instance *inst);

int checkTime(Instance *inst, bool saveBest);

#define CHECKTIME(inst, bool)                                                  \
  do {                                                                         \
    ErrorCode err = checkTime((inst), (bool));                                 \
    if (err != SUCCESS) {                                                      \
      return err;                                                              \
    }                                                                          \
  } while (0)

// ----------------------------------------------------------------------------
// General instance related utils
// ----------------------------------------------------------------------------

void swapPathPoints(Instance *inst, int i, int j);

int INSTANCE_pathCheckpoint(Instance *inst);

ErrorCode INSTANCE_assert(Instance *inst);
#define ASSERTINST(inst)                                                       \
  do {                                                                         \
    RUN(INSTANCE_assert(inst));                                                \
  } while (0)

#endif /* VRP_H_ */
