#ifndef METAHEURISTICS_H
#define METAHEURISTICS_H
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../include/logger.h"
#include "../include/refinement.h"
#include "../include/utils.h"
#include "../include/vrp.h"

/**
 * @brief Simulated annealing
 * @param distance_matrix Distance matrix
 * @param path Path
 * @param nnodes Number of nodes
 * @param tour_length Tour length
 * @param k_max Maximum number of iterations
 * @param duration Duration
 */
void simulate_anealling(Instance *inst, double k_max);

/**
 * @brief Energy probabilities
 * @param cost_current Current cost
 * @param cost_new New cost
 * @param T Temperature
 */
double energy_probabilities(double cost_current, double cost_new, double T,
                            double coefficient);

// --------------------------- VNS ---------------------------
void vns_k(Instance *inst, int k);

/** Kick function
 * @param distance_matrix: distance matrix
 * @param path: path to be filled
 * @param nnodes: number of nodes
 * @param tour_length: tour length
 * @param k: number of blocks
 */
void kick_function(Instance *inst, int k);
#endif
