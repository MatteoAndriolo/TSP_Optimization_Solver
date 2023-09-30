#ifndef HEURISTICS_H
#define HEURISTICS_H
#include <stdbool.h>

#include "../include/logger.h"
#include "../include/plot.h"
#include "../include/refinement.h"
#include "../include/utils.h"
#include "../include/vrp.h"


void vnp_k(Instance *inst, int k);

/** Kick function
 * @param distance_matrix: distance matrix
 * @param path: path to be filled
 * @param nnodes: number of nodes
 * @param tour_length: tour length
 * @param k: number of blocks
 */
void kick_function(Instance *inst, int k);


#endif
