#ifndef VRP_H_
#define VRP_H_

#include <cplex.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include "../tmpcplex/cplex.h"

#include "../include/errors.h"
#include "../include/genetic.h"
#include "../include/grasp.h"

#define XSMALL 1e-5
#define EPSILON 1e-9
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
  double max_time;  // overall time limit, in sec.s
  time_t tstart;    // starting time
  time_t tend;      // end time
  // pthread_mutex_t mut_calcTourLenght, mut_assert, mut_pathCheckpoint;

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
  double best_tourlength;  // best sol. available

  // GRASP parameters
  GRASP_Framework *grasp;

  // TSP MODEL
  CPXENVptr env;
  CPXLPptr lp;
  int ncols;
  TSPSolvers solver;

  int ecount;
  int *edgeList;  // pairs node to node for each edge in list
  double percentageLB;
  int percentageHF;
  void *params;

  // GENETIC parameters
  GENETIC_SETUP *genetic_setup;
  int ngenerations;
  int population_size;

  // global data
  int curr_iter;
  int *iterations;
  int *costs;
  double *times;
  char input_file[1000];  // input file
  char log_file[1000];    // output log file
} Instance;

void INSTANCE_initialize(Instance *inst, double max_time, int integer_costs,
                         int nnodes, double *x, double *y,
                         int grasp_n_probabilities, double *grasp_probabilities,
                         char *input_file, char *log_file, bool perfprof);

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

ErrorCode INSTANCE_setPath(Instance *inst, int *newPath);

ErrorCode checkTime(Instance *inst, bool saveBest);

#define CHECKTIME(inst, b) RUN(checkTime(inst, b));

// ----------------------------------------------------------------------------
// General instance related utils
// ----------------------------------------------------------------------------

void swapPathPoints(Instance *inst, int i, int j);

ErrorCode INSTANCE_pathCheckpoint(Instance *inst);

ErrorCode INSTANCE_assert(Instance *inst);

void INSTANCE_storeCost(Instance *inst, int iter);

void writeCosts(Instance *inst, FILE *fp);

// ----------------------------------------------------------------------------
// FROM GENETIC
Instance *temp_instance(Instance *inst, int *path);

double calculateTourLenghtPath(Instance *inst, int *path);
#endif /* VRP_H_ */
