#ifndef GREEDY_H
#define GREEDY_H
#include "vrp.h"
#include "plot.h"
#include "refinement.h"

/**
 * Implements the nearest neighbor heuristic to solve the TSP for the given instance.
 *
 * @param inst a pointer to the instance to be solved
 */
void nearest_neighboor(const double *distance_matrix, int *path, int nnodes, double *tour_length);

void nearest_neighboor_grasp(const double *distance_matrix, int *path, const int nnodes, double *tour_length, const double *probabilities, const int n_prob);
/**
 * Implements a variation of the extra mileage heuristic to solve the TSP for the given instance.
 *
 * @param distance_matrix a pointer to the distance matrix
 * @param path a pointer to the path
 * @param nnodes the number of nodes
 * @param tour_length a pointer to the tour length
 */
void extra_mileage(const double *distance_matrix, int *path, int nnodes, double *tour_length);

/**
 * Implements an updated version of the extra mileage heuristic to solve the TSP for the given instance.
 *
 * @param inst a pointer to the instance to be solved
 */
void updated_extra_mileage(Instance *inst);

#endif /* NEAREST_NEIGHBOR_H */
