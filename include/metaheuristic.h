#ifndef METAHEURISTICS_H
#define METAHEURISTICS_H
#include "../include/vrp.h"
#include "errors.h"

/**
 * @brief Simulated annealing
 *
 * @param inst Instance
 * @param k_max max jump
 */
int simulated_annealling(Instance *inst, double k_max);

/**
 * @brief Energy probabilities
 *
 * @param cost_current Current cost
 * @param cost_new New cost
 * @param T Temperature
 * @param coefficient Coefficient
 */
double energy_probabilities(double cost_current, double cost_new, double T,
                            double coefficient);

// --------------------------- VNS ---------------------------
/**
 * @brief vns_k
 *
 * @param inst: instance
 * @param start: max jump
 * @param end: min jump
 * @param iterations: number of iterations
 */
ErrorCode vns_k(Instance *inst, int start, int end, int iterations);

/**
 * @param distance_matrix: distance matrix
 *
 * @param inst: instance
 * @param k: number of blocks
 */
ErrorCode kick_function(Instance *inst, int k);
ErrorCode kick_function_tabu(Instance *inst, int k, TabuList *tabu);

// --------------------------- GENETIC  ---------------------------
ErrorCode genetic_algorithm(Instance *inst);
#endif
