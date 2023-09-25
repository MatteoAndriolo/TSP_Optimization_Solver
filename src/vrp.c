#include "../include/vrp.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

void initializeInstance(Instance *inst, int randomseed, double timelimit,
                        const char *input_file, const char *log_file,
                        int model_type, int integer_costs, int nnodes,
                        double *x, double *y, double *distance_matrix,
                        int node_start, double tour_length, int *path,
                        double zbest, int *path_best, int grasp_n_probabilities,
                        double *grasp_probabilities, double tstart,
                        double tend) {
  if (!inst) {
    return;
  }

  // Initialize execution parameters
  inst->randomseed = randomseed;
  inst->timelimit = timelimit;

  strncpy(inst->input_file, input_file, sizeof(inst->input_file) - 1);
  inst->input_file[sizeof(inst->input_file) - 1] = '\0';

  strncpy(inst->log_file, log_file, sizeof(inst->log_file) - 1);
  inst->log_file[sizeof(inst->log_file) - 1] = '\0';

  // Initialize MODEL
  inst->model_type = model_type;
  inst->integer_costs = integer_costs;

  // Initialize Data
  inst->nnodes = nnodes;
  inst->x = x;
  inst->y = y;
  inst->distance_matrix = distance_matrix;

  // Initialize Paths
  inst->node_start = node_start;
  inst->tour_length = tour_length;
  inst->path = path;
  inst->zbest = zbest;
  inst->path_best = path_best;

  // Initialize mutex
  pthread_mutex_init(&inst->mutex_path, NULL);

  // Initialize GRASP parameters
  inst->grasp_n_probabilities = grasp_n_probabilities;
  inst->grasp_probabilities = grasp_probabilities;

  // Initialize global data
  inst->tstart = tstart;
  inst->tend = tend;
}

double getDistancePos(Instance *inst, int x, int y) {
  return inst->distance_matrix[inst->path[x] * inst->nnodes + inst->path[y]];
}
double getDistanceNodes(Instance *inst, int x, int y) {
  return inst->distance_matrix[x * inst->nnodes + y];
}

void setTourLenght(Instance *inst, double newLength) {
  pthread_mutex_lock(&inst->mutex_path);
  inst->tour_length = newLength;
  pthread_mutex_unlock(&inst->mutex_path);
}

void addToTourLenght(Instance *inst, double toAdd) {
  setTourLenght(inst, inst->tour_length + toAdd);
}

void destroyInstance(Instance *inst) {
  if (inst) {
    // Free dynamically allocated memory
    if (inst->x) {
      free(inst->x);
      inst->x = NULL;
    }

    if (inst->y) {
      free(inst->y);
      inst->y = NULL;
    }

    if (inst->distance_matrix) {
      free(inst->distance_matrix);
      inst->distance_matrix = NULL;
    }

    if (inst->path) {
      free(inst->path);
      inst->path = NULL;
    }

    if (inst->path_best) {
      free(inst->path_best);
      inst->path_best = NULL;
    }

    if (inst->grasp_probabilities) {
      free(inst->grasp_probabilities);
      inst->grasp_probabilities = NULL;
    }

    // Destroy the mutex
    pthread_mutex_destroy(&inst->mutex_path);
  }
}

void swapPathPoints(Instance *inst, int i, int j) {
  pthread_mutex_lock(&inst->mutex_path);
  int temp = inst->path[i];
  inst->path[i] = inst->path[j];
  inst->path[j] = temp;
  pthread_mutex_unlock(&inst->mutex_path);
}

void swapAndShiftPath(Instance *inst, int i, int j) {
  pthread_mutex_lock(&inst->mutex_path);
  int temp =
      inst->path[j];  // Save the value at position j in a temporary variable
  for (int k = j; k > i; k--) {
    inst->path[k] =
        inst->path[k - 1];  // Shift all elements to the right from j to i+1
  }
  inst->path[i] = temp;  // Put the saved value from position j into position i
  pthread_mutex_unlock(&inst->mutex_path);
}

double calculateTourLength(Instance *inst) {
  pthread_mutex_lock(&inst->mutex_path);
  double tour_length = getDistancePos(inst, 0, inst->nnodes - 1);
  for (int i = 0; i < inst->nnodes - 1; i++) {
    tour_length += getDistancePos(inst, i, i + 1);
  }
  inst->tour_length = tour_length;
  pthread_mutex_unlock(&inst->mutex_path);
  return tour_length;
}

int assertInst(Instance *inst) {
  /*
   * Checks if the path is valid
   * 1. All nodes are used
   * 2. The tour length is correct
   */

  int check_nnodes = (inst->nnodes * (inst->nnodes - 1)) / 2;
  for (int i = 0; i < inst->nnodes; check_nnodes -= inst->path[i++])
    ;

  if (check_nnodes != 0) {
    ERROR_COMMENT("assertInst", "Not all nodes are used");
    return ERROR_NODES;
  }

  double check_tour_length = calculateTourLength(inst);
  if (check_tour_length != inst->tour_length) {
    return ERROR_TOUR_LENGTH;
  }

  return SUCCESS;
}
