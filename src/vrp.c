#include "../include/vrp.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/grasp.h"
#include "../include/utils.h"

void INSTANCE_generatePath(Instance *inst) {
  inst->path = malloc(inst->nnodes * sizeof(int));
  inst->best_path = malloc(inst->nnodes * sizeof(int));
  for (int i = 0; i < inst->nnodes; i++) {
    inst->path[i] = i;
  }
  inst->path[inst->starting_node] = 0;
  inst->path[0] = inst->starting_node;

  int j, tmp;
  for (int i = 0; i < inst->nnodes; i++) {
    j = rand() % inst->nnodes;
    tmp = inst->path[i];
    inst->path[i] = inst->path[j];
    inst->path[j] = tmp;
  }
  memcpy(inst->best_path, inst->path, inst->nnodes * sizeof(int));
  inst->tour_length = INSTANCE_calculateTourLength(inst);
  inst->best_tourlength = inst->tour_length;
}

void INSTANCE_generateDistanceMatrix(Instance *inst) {
  double *dm = malloc(inst->nnodes * inst->nnodes * sizeof(double));
  for (int i = 0; i < inst->nnodes; i++) {
    for (int j = 0; j < inst->nnodes; j++) {
      dm[i * inst->nnodes + j] = distance_euclidean(
          inst->x[i], inst->y[i], inst->x[j],
          inst->y[j]); // DEBUG_COMMENT("instance_generate_distance_matrix",
                       // "Distance from %d (%lf, %lf) to %d (%lf, %lf)  :
                       // %lf", i, j, inst->x[i], inst->y[i],inst->x[j],
                       // inst->y[j],dm[i * inst->nnodes + j]);
    }
  }
  for (int i = 0; i < inst->nnodes; i++) {
    dm[i * inst->nnodes + i] = INFINITY;
  }
  inst->distance_matrix = dm;
}

void INSTANCE_initialize(Instance *inst, double max_time, int integer_costs,
                         int nnodes, double *x, double *y,
                         int grasp_n_probabilities, double *grasp_probabilities,
                         char *input_file, char *log_file) {
  INFO_COMMENT("vrp.c:instance_initialize", "Initializing instance");
  if (!inst) {
    return;
  }

  // Initialize execution parameters
  inst->max_time = max_time;
  inst->tstart = time(NULL);
  inst->tend = inst->tstart + max_time;

  // Initialize MODEL
  // inst->model_type = model_type;
  inst->integer_costs = integer_costs;

  // Initialize Data
  inst->nnodes = nnodes;
  inst->starting_node = rand() % nnodes;
  // memcpy x inside inst->x of dimension inst->nnodes
  inst->x = malloc(sizeof(double) * inst->nnodes);
  inst->y = malloc(sizeof(double) * inst->nnodes);
  memcpy(inst->x, x, sizeof(double) * inst->nnodes);
  memcpy(inst->y, y, sizeof(double) * inst->nnodes);
  /* generate distance matrix */
  INSTANCE_generateDistanceMatrix(inst);
  /*
   * generate path -> store in path and backup in path_best
   * calulate tour lenght -> init tour lenght and best_tour
   */
  INSTANCE_generatePath(inst);

  // Initialize mutex
  pthread_mutex_init(&inst->mutex_path, NULL);

  // Initialize GRASP parameters
  inst->grasp = (GRASP_Framework *)malloc(sizeof(GRASP_Framework));
  GRASP_init(inst->grasp, grasp_probabilities, grasp_n_probabilities);

  inst->grasp->size = grasp_n_probabilities;
  inst->grasp->probabilities = grasp_probabilities;
  // inst->grasp_n_probabilities = grasp_n_probabilities;
  // inst->grasp_probabilities = malloc(grasp_n_probabilities * sizeof(double));
  // memcpy(inst->grasp_probabilities, grasp_probabilities,
  //         grasp_n_probabilities * sizeof(double));

  // Initialize file names
  strncpy(inst->input_file, input_file, sizeof(inst->input_file) - 1);
  inst->input_file[sizeof(inst->input_file) - 1] = '\0';

  strncpy(inst->log_file, log_file, sizeof(inst->log_file) - 1);
  inst->log_file[sizeof(inst->log_file) - 1] = '\0';
  INFO_COMMENT("vrp.c:instance_initialize", "Instance initialized");
}

void INSTANCE_free(Instance *inst) {
  if (inst) {
    if (inst->path) {
      free(inst->path);
      inst->path = NULL;
      DEBUG_COMMENT("vrp.c:instance_destroy", "path");
    }

    if (inst->distance_matrix) {
      free(inst->distance_matrix);
      inst->distance_matrix = NULL;
      DEBUG_COMMENT("vrp.c:instance_destroy", "distance_matrix");
    }

    if (inst->x) {
      free(inst->x);
      inst->x = NULL;
      DEBUG_COMMENT("vrp.c:instance_destroy", "x");
    }

    if (inst->y) {
      free(inst->y);
      inst->y = NULL;
      DEBUG_COMMENT("vrp.c:instance_destroy", "y");
    }

    if (inst->best_path) {
      free(inst->best_path);
      inst->best_path = NULL;
      DEBUG_COMMENT("vrp.c:instance_destroy", "best_path");
    }

    // if (inst->grasp_probabilities) {
    //     free(inst->grasp_probabilities);
    //     inst->grasp_probabilities = NULL;
    //     DEBUG_COMMENT("vrp.c:instance_destroy", "grasp_probabilities");
    // }

    // Destroy the mutex
    if (pthread_mutex_destroy(&inst->mutex_path) != 0) {
      ERROR_COMMENT("vrp.c:instance_destroy", "mutex destroy failed");
    }

    if (inst->edgeList != NULL) {
      free(inst->edgeList);
      inst->edgeList = NULL;
      DEBUG_COMMENT("vrp.c:instance_destroy", "edgeList");
    }

    // pthread_mutex_destroy(&inst->mutex_path);
    DEBUG_COMMENT("vrp.c:instance_destroy", "mutex destroyed");
  } else {
    ERROR_COMMENT("vrp.c:instance_destroy", "inst is NULL");
  }
}

/*
 * Returns the distance of the tour (inst->path)
 * Updates the tour lenght of the instance (inst->tour_lenght)
 *
 * @param inst The instance of the problem to be solved.
 * @return the tour lenght
 */
double INSTANCE_calculateTourLength(Instance *inst) {
  double tour_length = INSTANCE_getDistancePos(inst, 0, inst->nnodes - 1);
  for (int i = 0; i < inst->nnodes - 1; i++) {
    tour_length += INSTANCE_getDistancePos(inst, i, i + 1);
  }
  inst->tour_length = tour_length;
  return tour_length;
}

/*
 * Returns the distance of the tour (inst->path) given two positions in path
 *
 * @param inst The instance of the problem to be solved.
 * @param x The first position in path_best
 * @param y The second position in path_best
 * @return the distance between the two nodes
 */
double INSTANCE_getDistancePos(Instance *inst, int x, int y) {
  return inst->distance_matrix[inst->path[x] * inst->nnodes + inst->path[y]];
}

/*
 * Returns the distance of the tour (inst->path) given two nodes
 *
 * @param inst The instance of the problem to be solved.
 * @param x The first nodes
 * @param y The second nnodes
 * @return the distance between the two nodes
 */
double INSTANCE_getDistanceNodes(Instance *inst, int x, int y) {
  return inst->distance_matrix[x * inst->nnodes + y];
}

/*
 * Set the tour lenght of the instance
 *
 * @param inst The instance of the problem to be solved.
 * @param newLength The new tour lenght
 */
void INSTANCE_setTourLenght(Instance *inst, double newLength) {
  inst->tour_length = newLength;
}

/*
 * Add to the tour lenght of the instance
 *
 * @param inst The instance of the problem to be solved.
 * @param toAdd The value to add to the tour lenght
 */
void INSTANCE_addToTourLenght(Instance *inst, double toAdd) {
  INSTANCE_setTourLenght(inst, inst->tour_length + toAdd);
}

/*
 * Checks if path is correct:
 * 1. All nodes are used -> ERROR_NODES
 * 2. THe tour length is correct -> ERROR_TOUR_LENGTH
 *
 * @param inst The instance of the problem to be solved.
 * @return SUCCESS if the path is correct, ERROR_NODES or ERROR_TOUR_LENGTH
 */
ErrorCode INSTANCE_assert(Instance *inst) {
  DEBUG_COMMENT("vrp.c:INSTANCE_assert", "Checking instance");

  // ------ NODES
  int check_nnodes = (inst->nnodes * (inst->nnodes - 1)) / 2;
  for (int i = 0; i < inst->nnodes; i++) {
    check_nnodes -= inst->path[i];
    if (inst->path[i] < 0 || inst->path[i] >= inst->nnodes) {
      ERROR_COMMENT("vrp.c:INSTANCE_assert", "Node %d is not valid",
                    inst->path[i]);
      return ERROR_NODES;
    }
  }

  if (check_nnodes != 0) {
    ERROR_COMMENT("vrp.c:INSTANCE_assert", "Missing %d nodes", check_nnodes);
    return ERROR_NODES;
  }

  // ------ TOUR LENGTH
  double check_tour_length = INSTANCE_calculateTourLength(inst);
  if (check_tour_length != inst->tour_length) {
    ERROR_COMMENT("vrp.c:INSTANCE_assert", "Tour length is not correct",
                  check_tour_length - inst->tour_length);
    return ERROR_TOUR_LENGTH;
  }

  DEBUG_COMMENT("vrp.c:INSTANCE_assert", "Instance is valid");
  return SUCCESS;
}

/*
 * Saves path as the best path if it is better than the current best path
 * after hacing checked it
 *
 * @param inst The instance of the problem to be solved.
 * @return SUCCESS if the path is correct, ERROR_NODES or ERROR_TOUR_LENGTH
 */
int INSTANCE_pathCheckpoint(Instance *inst) {
  DEBUG_COMMENT("vrp.c:INSTANCE_pathCheckpoint", "Is this path the best?");

  DEBUG_COMMENT("vrp.c:INSTANCE_pathCheckpoint", "RUN calculateTourLength");
  RUN(INSTANCE_calculateTourLength(inst));

  DEBUG_COMMENT("vrp.c:INSTANCE_pathCheckpoint", "RUN assert");
  ASSERTINST(inst);

  if (inst->tour_length < inst->best_tourlength) {
    DEBUG_COMMENT("vrp.c:INSTANCE_pathCheckpoint", "This path is the best");
    memcpy(inst->best_path, inst->path, inst->nnodes * sizeof(int));
    inst->best_tourlength = inst->tour_length;
    INFO_COMMENT("vrp.c:pathCheckpoint", "New best path found: %lf",
                 inst->best_tourlength);
  } else {
    INFO_COMMENT("vrp.c:INSTANCE_pathCheckpoint", "This path is not the best");
    DEBUG_COMMENT("vrp.c:INSTANCE_pathCheckpoint",
                  "Best path: %lf, Current path: %lf", inst->best_tourlength,
                  inst->tour_length);
  }
  DEBUG_COMMENT("vrp.c:INSTANCE_pathCheckpoint", "Path checkpoint done");
  return SUCCESS;
}

void INSTANCE_saveBestPath(Instance *inst) { INSTANCE_pathCheckpoint(inst); }

/*
 * Checks if the time limit has been reached
 * If saveBest is true, it saves the best path
 *
 * @param inst The instance of the problem to be solved.
 * @param saveBest If true, it saves the best path
 * @return SUCCESS if the path is correct, ERROR_NODES or ERROR_TOUR_LENGTH
 */
int checkTime(Instance *inst, bool saveBest) {
  if (inst->tend < time(NULL)) {
    if (saveBest) {
      RUN(INSTANCE_pathCheckpoint(inst));
    }
    ERROR_COMMENT("vrp.c:checkTime", "Time limit reached");
    return ERROR_TIME_LIMIT;
  }
  return OK;
}
