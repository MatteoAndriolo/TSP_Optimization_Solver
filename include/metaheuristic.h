#ifndef METAHEURISTICS_H
#define METAHEURISTICS_h
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "vrp.h"
#include "refinement.h"
#include "utils.h"
#include "logger.h"
#include "heuristics.h"

/**
 * @brief Simulated annealing metaheuristic
 * @param distance_matrix Distance matrix
 * @param path Path to be optimized
 * @param nnodes Number of nodes
 * @param tour_length Length of the path
 * @param T temperature 
 * @param duration Duration of the algorithm
*/
void simulate_anealling(const double *distance_matrix, int *path, int nnodes, double *tour_length, double T, int duration);

/**
* @brief Random number between two numbers
* @param lowerBound Lower bound
* @param upperBound Upper bound
*/
double randomBetween_d(double lowerBound, double upperBound);

#endif