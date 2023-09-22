#ifndef METAHEURISTICS_H
#define METAHEURISTICS_h
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../include/heuristics.h"
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
void simulate_anealling(const double *distance_matrix, int *path, int nnodes,
                        double *tour_length, double k_max, int duration);

/**
 * @brief Random number between two numbers
 * @param lowerBound Lower bound
 * @param upperBound Upper bound
 */
double randomBetween_d(double lowerBound, double upperBound);

/**
 * @brief Energy probabilities
 * @param cost_current Current cost
 * @param cost_new New cost
 * @param T Temperature
 */
double energy_probabilities(double cost_current, double cost_new, double T,
                            double coefficient);

#endif