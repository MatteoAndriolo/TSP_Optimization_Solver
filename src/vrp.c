#include "../include/vrp.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/utils.h"

void instance_generate_path(Instance *inst) {
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
}

void instance_generate_distance_matrix(Instance *inst) {
    double* dm= malloc(inst->nnodes * inst->nnodes * sizeof(double));
    for (int i = 0; i < inst->nnodes; i++) {
        for (int j = 0; j < inst->nnodes; j++) {
            dm[i * inst->nnodes + j] = round(
                    distance_euclidean(inst->x[i], inst->y[i], inst->x[j], inst->y[j]));
        }
    }
    for (int i = 0; i < inst->nnodes; i++) {
        dm[i * inst->nnodes + i] = INFINITY;
    }
    inst->distance_matrix = dm;
}

void instance_initialize(Instance *inst, double max_time,
        int integer_costs, int nnodes,
        double *x, double *y, int grasp_n_probabilities,
        double *grasp_probabilities, char *input_file,
        char *log_file) {
    INFO_COMMENT("instance_initialize", "Initializing instance");
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
    //memcpy x inside inst->x of dimension inst->nnodes
    inst->x = malloc(sizeof(double)*inst->nnodes);
    inst->y = malloc(sizeof(double)*inst->nnodes);
    memcpy(inst->x,x,sizeof(int)*inst->nnodes);
    memcpy(inst->y,y,sizeof(int)*inst->nnodes);
    /* generate distance matrix */
    instance_generate_distance_matrix(inst);
    /*
     * generate path -> store in path and backup in path_best
     * calulate tour lenght -> init tour lenght and best_tour
     */
    instance_generate_path(inst);

    // Initialize mutex
    pthread_mutex_init(&inst->mutex_path, NULL);

    // Initialize GRASP parameters
    inst->grasp_n_probabilities = grasp_n_probabilities;
    inst->grasp_probabilities = malloc(grasp_n_probabilities * sizeof(double));
    memcpy(inst->grasp_probabilities, grasp_probabilities,
            grasp_n_probabilities * sizeof(double));

    // Initialize file names
    strncpy(inst->input_file, input_file, sizeof(inst->input_file) - 1);
    inst->input_file[sizeof(inst->input_file) - 1] = '\0';

    strncpy(inst->log_file, log_file, sizeof(inst->log_file) - 1);
    inst->log_file[sizeof(inst->log_file) - 1] = '\0';
    INFO_COMMENT("instance_initialize", "Instance initialized");
}


void instance_destroy(Instance *inst) {
    if (inst) {
        if (inst->path) {
            free(inst->path);
            inst->path = NULL;
            DEBUG_COMMENT("instance_destroy", "path");
        }

        if (inst->distance_matrix) {
            free(inst->distance_matrix);
            inst->distance_matrix = NULL;
            DEBUG_COMMENT("instance_destroy", "distance_matrix");
        }

        if (inst->x) {
            free(inst->x);
            inst->x = NULL;
            DEBUG_COMMENT("instance_destroy", "x");
        }

        if (inst->y) {
            free(inst->y);
            inst->y = NULL;
            DEBUG_COMMENT("instance_destroy", "y");
        }

        if (inst->best_path) {
            free(inst->best_path);
            inst->best_path = NULL;
            DEBUG_COMMENT("instance_destroy", "best_path");
        }


        if (inst->grasp_probabilities) {
            free(inst->grasp_probabilities);
            inst->grasp_probabilities = NULL;
            DEBUG_COMMENT("instance_destroy", "grasp_probabilities");
        }

        // Destroy the mutex
        if (pthread_mutex_destroy(&inst->mutex_path) != 0) {
            ERROR_COMMENT("instance_destroy", "mutex destroy failed");
        }
        //pthread_mutex_destroy(&inst->mutex_path);
        DEBUG_COMMENT("instance_destroy", "mutex destroyed");
    }
    else{
        ERROR_COMMENT("instance_destroy", "inst is NULL");
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
