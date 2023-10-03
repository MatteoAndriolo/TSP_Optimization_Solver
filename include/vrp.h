#ifndef VRP_H
#define VRP_H

#include <getopt.h>
#include <math.h>
// #include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// #include <cplex.h>
#include <pthread.h>

#include "errors.h"
#include "genetic.h"
#include "grasp.h"
#include "logger.h"
// hard-wired parameters
#define XSMALL \
  1e-5  // 1e-4*	// tolerance used to decide ingerality of 0-1 var.s
#define EPSILON 1e-9  // 1e-9		// very small numerical tolerance
#define TICKS_PER_SECOND \
  1000.0  // cplex's ticks on Intel Core i7 quadcore @2.3GHZ

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
  double best_tourlength;  // best sol. available

  // GRASP parameters
  // int grasp_n_probabilities;
  // double *grasp_probabilities;
  GRASP_Framework *grasp;

  // GENETIC parameters
  GENETIC_SETUP genetic_setup;

  // global data
  char input_file[1000];  // input file
  char log_file[1000];    // output log file
} Instance;

void instance_initialize(Instance *inst, double max_time,
                         int integer_costs, int nnodes,
                         double *x,double *y, int grasp_n_probabilities,
                         double *grasp_probabilities, char *input_file,
                         char *log_file);

Instance* temp_instance(Instance *inst, int* path);

void instance_destroy(Instance *inst);

void instance_generate_path(Instance *inst);

void instance_generate_distance_matrix(Instance *inst);

// ----------------------------------------------------------------------------
// Distance related utils
// ----------------------------------------------------------------------------
double getDistancePos(Instance *inst, int x, int y);

double getDistanceNodes(Instance *inst, int x, int y);

double calculateTourLength(Instance *inst);
double calculateTourLenghtPath(Instance *inst, int* path);

void setTime(Instance *inst,time_t time);

void setTourLenght(Instance *inst, double newLength);

void addToTourLenght(Instance *inst, double toAdd);

void saveBestPath(Instance *inst);

int checkTime(Instance *inst, bool saveBest);

#define CHECKTIME(inst, bool) \
    do { \
        ErrorCode err= checkTime((inst), (bool)); \
        if( err != SUCCESS) { \
            return err; \
        } \
    } while (0)

// ----------------------------------------------------------------------------
// General instance related utils
// ----------------------------------------------------------------------------

void swapPathPoints(Instance *inst, int i, int j);

int pathCheckpoint(Instance *inst);

int assertInst(Instance *inst);
#define ASSERTINST(inst) \
    do { \
        RUN(assertInst(inst)); \
    } while (0)



#endif /* VRP_H_ */
