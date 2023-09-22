#ifndef HEURISTICS_H
#define HEURISTICS_H
#include <stdbool.h>

#include "../include/logger.h"
#include "../include/plot.h"
#include "../include/refinement.h"
#include "../include/utils.h"
#include "../include/vrp.h"

/**
 * Individual structure used for single specimen in the genetic algorithm
 */
typedef struct {
  int *path;
  double fitness;
} Individual;

/** Nearest Neighbor Heuristic
 * @param distance_matrix: distance matrix
 * @param path: path to be filled
 * @param nnodes: number of nodes
 * @param tour_length: tour length
 */

void vnp_k(const double *distance_matrix, int *path, int nnodes,
           double *tour_length, int k, int duration);

/** Kick function
 * @param distance_matrix: distance matrix
 * @param path: path to be filled
 * @param nnodes: number of nodes
 * @param tour_length: tour length
 * @param k: number of blocks
 */
void kick_function(const double *distance_matrix, int *path, int nnodes,
                   double *tour_length, int k);
/**
 * @param
 */
int randomBetween(int lowerBound, int upperBound);

#endif