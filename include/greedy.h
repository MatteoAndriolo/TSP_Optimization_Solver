#ifndef GREEDY_H
#define GREEDY_H
#include <math.h>
#include <stdlib.h>
#include "vrp.h"
#include "errors.h"

/**
 * Implements the nearest neighbor heuristic to solve the TSP for the given
 * instance.
 *
 * @param distance_matrix a pointer to the distance matrix
 * @param path a pointer to the path
 * @param nnodes the number of nodes
 * @param tour_length a pointer to the tour length
 */
int nearest_neighboor(Instance *inst);

/**
 * Implements the nearest neighbor heuristic to solve the TSP for the given
 * instance.
 *
 * @param distance_matrix a pointer to the distance matrix
 * @param path a pointer to the path
 * @param nnodes the number of nodes
 * @param tour_length a pointer to the tour length
 * @param probabilities a pointer to the probabilities array
 * @param n_prob the number of probabilities
 */
int nearest_neighboor_grasp(Instance *inst);

/**
 * Implements a variation of the extra mileage heuristic to solve the TSP for
 * the given instance.
 *
 * @param distance_matrix a pointer to the distance matrix
 * @param path a pointer to the path
 * @param nnodes the number of nodes
 * @param tour_length a pointer to the tour length
 */
int extra_mileage(Instance *inst);

#endif /* NEAREST_NEIGHBOR_H */
