#ifndef GREEDY_H
#define GREEDY_H
#include "vrp.h"


/**
 * Implements the nearest neighbor heuristic to solve the TSP for the given instance.
 * 
 * @param inst a pointer to the instance to be solved
 * @param instances the number of instances to be solved
 */
void model_nearest_neighboor(Instance *inst, int instances);

/**
 * Implements the nearest neighbor heuristic to solve the TSP for the given instance.
 * 
 * @param inst a pointer to the instance to be solved
 */
void nearest_neighboor(double *distance_matrix, int *path, int nnodes, int starting_node, double *tour_length);

/**
 * Implements a variation of the extra mileage heuristic to solve the TSP for the given instance.
 * 
 * @param inst a pointer to the instance to be solved
 * @param instances the number of instances to be solved
 */
void extra_mileage(Instance *inst, int instances);

/**
 * Implements an updated version of the extra mileage heuristic to solve the TSP for the given instance.
 * 
 * @param inst a pointer to the instance to be solved
 */
void updated_extra_mileage(Instance *inst);



#endif /* NEAREST_NEIGHBOR_H */
