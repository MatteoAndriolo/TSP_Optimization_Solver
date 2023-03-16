#ifndef GREEDY_H
#define GREEDY_H
#include "vrp.h"
#include "logger.h"
#include "utils.h"
#include "logger.h"
#include <stdlib.h>
#include <math.h>


/**
 * Implements the nearest neighbor heuristic to solve the TSP for the given instance.
 * 
 * @param inst a pointer to the instance to be solved
 */
void model_nearest_neighboor(Instance *inst);

/**
 * Implements a variation of the extra mileage heuristic to solve the TSP for the given instance.
 * 
 * @param inst a pointer to the instance to be solved
 */
void extra_mileage(Instance *inst);

/**
 * Implements an updated version of the extra mileage heuristic to solve the TSP for the given instance.
 * 
 * @param inst a pointer to the instance to be solved
 */
void updated_extra_mileage(Instance *inst);



#endif /* NEAREST_NEIGHBOR_H */
