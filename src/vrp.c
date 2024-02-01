// CPXWRITE
#include "../include/vrp.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/grasp.h"
#include "../include/utils.h"

void INSTANCE_generatePath(Instance *inst) {
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
          inst->y[j]);  // DEBUG_COMMENT("instance_generate_distance_matrix",
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
                         char *input_file, char *log_file, bool perfprof) {
  INFO_COMMENT("vrp.c:instance_initialize", "Initializing instance");
  if (!inst) {
    return;
  }
  // Initialize execution parameters
  inst->max_time = max_time;
  inst->tstart = clock();
  inst->tend = inst->tstart + max_time;
  printf("tstart: %ld\n", inst->tstart);

  // Initialize MODEL
  // inst->model_type = model_type;
  inst->integer_costs = integer_costs;

  printf("integer_costs: %d\n", inst->integer_costs);

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
  inst->path = (int *)malloc(inst->nnodes * sizeof(int));
  inst->best_path = (int *)malloc(inst->nnodes * sizeof(int));
  INSTANCE_generatePath(inst);

  // Initialize mutex
  //  pthread_mutex_init(&inst->mut_assert, NULL);
  //  pthread_mutex_init(&inst->mut_calcTourLenght, NULL);
  //  pthread_mutex_init(&inst->mut_pathCheckpoint, NULL);

  // Initialize GRASP parameters
  inst->grasp = (GRASP_Framework *)malloc(sizeof(GRASP_Framework));
  GRASP_init(inst->grasp, grasp_probabilities, grasp_n_probabilities);

  inst->grasp->size = grasp_n_probabilities;
  inst->grasp->probabilities = grasp_probabilities;
  // inst->grasp_n_probabilities = grasp_n_probabilities;
  // inst->grasp_probabilities = malloc(grasp_n_probabilities * sizeof(double));
  // memcpy(inst->grasp_probabilities, grasp_probabilities,
  //         grasp_n_probabilities * sizeof(double));
  inst->edgeList = NULL;

  // Initialize file names
  strncpy(inst->input_file, input_file, sizeof(inst->input_file) - 1);
  inst->input_file[sizeof(inst->input_file) - 1] = '\0';

  strncpy(inst->log_file, log_file, sizeof(inst->log_file) - 1);
  inst->log_file[sizeof(inst->log_file) - 1] = '\0';

  DEBUG_COMMENT("vrp.c:instance_initialize", "tstart: %ld", inst->tstart);
  DEBUG_COMMENT("vrp.c:instance_initialize", "max_time: %lf", max_time);
  DEBUG_COMMENT("vrp.c:instance_initialize", "tend: %ld", inst->tend);
  DEBUG_COMMENT("vrp.c:instance_initialize", "integer_costs: %d",
                integer_costs);
  DEBUG_COMMENT("vrp.c:instance_initialize", "nnodes: %d", nnodes);
  DEBUG_COMMENT("vrp.c:instance_initialize", "starting_node: %d",
                inst->starting_node);
  INFO_COMMENT("vrp.c:instance_initialize", "Instance initialized");
}

void INSTANCE_free(Instance *inst) {
  if (inst) {
    if (inst->path) {
      free(inst->path);
      inst->path = NULL;
      DEBUG_COMMENT("vrp.c:INSTANCE_free", "path");
    }

    if (inst->distance_matrix) {
      free(inst->distance_matrix);
      inst->distance_matrix = NULL;
      DEBUG_COMMENT("vrp.c:INSTANCE_free", "distance_matrix");
    }

    if (inst->x) {
      free(inst->x);
      inst->x = NULL;
      DEBUG_COMMENT("vrp.c:INSTANCE_free", "x");
    }

    if (inst->y) {
      free(inst->y);
      inst->y = NULL;
      DEBUG_COMMENT("vrp.c:INSTANCE_free", "y");
    }

    if (inst->best_path) {
      free(inst->best_path);
      inst->best_path = NULL;
      DEBUG_COMMENT("vrp.c:INSTANCE_free", "best_path");
    }

    // if (inst->grasp_probabilities) {
    //     free(inst->grasp_probabilities);
    //     inst->grasp_probabilities = NULL;
    //     DEBUG_COMMENT("vrp.c:INSTANCE_free", "grasp_probabilities");
    // }

    //    // Destroy the mutex
    //    if (pthread_mutex_destroy(&inst->mut_pathCheckpoint) != 0 ||
    //        pthread_mutex_destroy(&inst->mut_calcTourLenght) != 0 ||
    //        pthread_mutex_destroy(&inst->mut_assert) != 0) {
    //      ERROR_COMMENT("vrp.c:INSTANCE_free", "mutex destroy failed");
    //    }

    if (inst->distance_matrix) {
      DEBUG_COMMENT("vrp.c:INSTANCE_free", "distance_matrix");
      free(inst->distance_matrix);
      inst->distance_matrix = NULL;
    }

    if (inst->edgeList != NULL) {
      DEBUG_COMMENT("vrp.c:INSTANCE_free", "edgeList");
      free(inst->edgeList);
      inst->edgeList = NULL;
    }

    DEBUG_COMMENT("vrp.c:INSTANCE_free", "mutex destroyed");
  } else {
    ERROR_COMMENT("vrp.c:INSTANCE_free", "inst is NULL");
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
  //  pthread_mutex_unlock(&inst->mut_calcTourLenght);
  //  pthread_mutex_lock(&inst->mut_calcTourLenght);
  double tour_length = INSTANCE_getDistancePos(inst, 0, inst->nnodes - 1);
  for (int i = 0; i < inst->nnodes - 1; i++) {
    tour_length += INSTANCE_getDistancePos(inst, i, i + 1);
  }
  inst->tour_length = tour_length;
  // pthread_mutex_unlock(&inst->mut_calcTourLenght);
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
  // pthread_mutex_lock(&inst->mut_assert);
  DEBUG_COMMENT("vrp.c:INSTANCE_assert", "Enter assert");
  bool found[inst->nnodes];
  for (int i = 0; i < inst->nnodes; i++) found[i] = false;

  // ------ NODES
  int check_nnodes = (inst->nnodes * (inst->nnodes - 1)) / 2;
  for (int i = 0; i < inst->nnodes; i++) {
    check_nnodes -= inst->path[i];
    found[inst->path[i]] = true;
    if (inst->path[i] < 0) {
      ERROR_COMMENT("vrp.c:INSTANCE_assert", "TourLenght is wrong");
      // pthread_mutex_unlock(&inst->mut_assert);
      return ERROR_TOUR_LENGTH;
    }
    if (inst->path[i] >= inst->nnodes) {
      ERROR_COMMENT("vrp.c:INSTANCE_assert", "Node is not valid");
      // pthread_mutex_unlock(&inst->mut_assert);
      return ERROR_NODES;
    }
  }

  bool isMissing = false;
  for (int i = 0; i < inst->nnodes; i++) {
    if (!found[i]) {
      isMissing = true;
      ERROR_COMMENT("vrp.c:INSTANCE_assert", "Missing node %d ", i);
    }
  }
  if (isMissing) {
    // pthread_mutex_unlock(&inst->mut_assert);
    return ERROR_NODES;
  }

  // ------ TOUR LENGTH
  double check_tour_length = INSTANCE_calculateTourLength(inst);
  if (check_tour_length != inst->tour_length) {
    ERROR_COMMENT("vrp.c:INSTANCE_assert", "Tour length is not correct",
                  check_tour_length - inst->tour_length);
    // pthread_mutex_unlock(&inst->mut_assert);
    return ERROR_TOUR_LENGTH;
  }

  DEBUG_COMMENT("vrp.c:INSTANCE_assert", "Exit assert");
  // pthread_mutex_unlock(&inst->mut_assert);
  return SUCCESS;
}

/*
 * Saves path as the best path if it is better than the current best path
 * after hacing checked it
 *
 * @param inst The instance of the problem to be solved.
 * @return SUCCESS if the path is correct, ERROR_NODES or ERROR_TOUR_LENGTH
 */
ErrorCode INSTANCE_pathCheckpoint(Instance *inst) {
  // pthread_mutex_lock(&inst->mut_pathCheckpoint);
  INFO_COMMENT("vrp.c:INSTANCE_pathCheckpoint", "Entering ");

  INSTANCE_calculateTourLength(inst);

  RUN(INSTANCE_assert(inst));
  if (inst->tour_length < inst->best_tourlength) {
    memcpy(inst->best_path, inst->path, inst->nnodes * sizeof(int));
    inst->best_tourlength = inst->tour_length;
    DEBUG_COMMENT("vrp.c:pathCheckpoint", "New best path found: %lf",
                  inst->best_tourlength);
  } else {
    DEBUG_COMMENT("vrp.c:INSTANCE_pathCheckpoint",
                  "Current path: %lf,Best path: %lf", inst->tour_length,
                  inst->best_tourlength);
  }

  INFO_COMMENT("vrp.c:INSTANCE_pathCheckpoint", "Exit ");
  // pthread_mutex_unlock(&inst->mut_pathCheckpoint);
  return SUCCESS;
}

/*
 * Saves path as the best path
 *
 * @param inst The instance of the problem to be solved.
 */
ErrorCode INSTANCE_saveBestPath(Instance *inst) {
  RUN(INSTANCE_pathCheckpoint(inst));
  return SUCCESS;
}

/*
 * Sets the path of the instance
 *
 * @param inst The instance of the problem to be solved.
 * @param newPath The new path
 */
ErrorCode INSTANCE_setPath(Instance *inst, int *newPath) {
  FREE(inst->path);
  inst->path = newPath;
  inst->tour_length = INSTANCE_calculateTourLength(inst);
  inst->best_tourlength = inst->tour_length;
  return SUCCESS;
}

/*
 * Checks if the time limit has been reached
 * If saveBest is true, it saves the best path
 *
 * @param inst The instance of the problem to be solved.
 * @param saveBest If true, it saves the best path
 * @return SUCCESS if the path is correct, ERROR_NODES or ERROR_TOUR_LENGTH
 */
ErrorCode checkTime(Instance *inst, bool saveBest) {
  if (inst->tend < clock()) {
    if (saveBest) {
      RUN(INSTANCE_pathCheckpoint(inst));
    }
    ERROR_COMMENT("vrp.c:checkTime", "Time limit reached");
    return ERROR_TIME_LIMIT;
  }

  return OK;
}

// FROM GENETIC

double calculateTourLenghtPath(Instance *inst, int *path) {
  double tour_length =
      inst->distance_matrix[path[0] * inst->nnodes + path[inst->nnodes - 1]];
  for (int i = 0; i < inst->nnodes - 1; i++) {
    tour_length += inst->distance_matrix[path[i] * inst->nnodes + path[i + 1]];
  }
  return tour_length;
}

Instance *temp_instance(Instance *inst, int *path) {
  Instance *I = malloc(sizeof(Instance));
  I->nnodes = inst->nnodes;
  I->starting_node = inst->starting_node;
  I->x = inst->x;
  I->y = inst->y;
  I->distance_matrix = inst->distance_matrix;
  I->path = path;
  I->tour_length = calculateTourLenghtPath(inst, path);
  I->best_path = I->path;
  I->best_tourlength = I->tour_length;
  I->grasp = inst->grasp;

  return I;
}

void writeCosts(Instance *inst) {
  FILE *fp;
  fp = fopen("costs.txt", "w");
  for (int i = 0; i < inst->ngenerations; i++) {
    fprintf(fp, "%d; %d; %lf\n", i, inst->costs[i], inst->times[i]);
  }
  fclose(fp);
}
