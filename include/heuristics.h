#ifndef HEURISTICS_H
#define HEURISTICS_h
#include "vrp.h"
#include "plot.h"
#include "refinement.h"
#include "utils.h"
#include "logger.h"

/** Nearest Neighbor Heuristic
    * @param distance_matrix: distance matrix
    * @param path: path to be filled
    * @param nnodes: number of nodes
    * @param tour_length: tour length
**/

void vnp_k(const double *distance_matrix, int *path, int nnodes, double *tour_length, int k, int duration);

/** Kick function
    * @param distance_matrix: distance matrix
    * @param path: path to be filled
    * @param nnodes: number of nodes
    * @param tour_length: tour length
    * @param k: number of blocks
**/

void kick_function(const double *distance_matrix, int *path, int nnodes, double *tour_length, int k);

#endif