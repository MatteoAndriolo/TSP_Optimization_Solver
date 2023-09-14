#ifndef REFINEMENT_H
#define REFINEMENT_H
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include "logger.h"
#include "utils.h"
#include "tabu.h"

#include "greedy.h"

/**
 * Applies the 2-opt algorithm to a given distance matrix and node list (in place).
 *
 * @param matrix A pointer to the distance matrix.
 * @param size_node The number of nodes in the tour.
 * @param node A pointer to the array of nodes in the tour.
 * @param tour_lenght A pointer to the tour length.
 * @param iterations The number of iterations to perform. (INFINITY: until no improvement is found)
 * @return void
 */
void two_opt(const double *matrix, int size_node, int *path, double *tour_lenght, double iterations);

/**
 * Applies the 2-opt algorithm to a given distance matrix and node list (in place), using a tabu list.
 *
 * @param matrix A pointer to the distance matrix.
 * @param size_node The number of nodes in the tour.
 * @param node A pointer to the array of nodes in the tour.
 * @param tour_lenght A pointer to the tour length.
 * @param iterations The number of iterations to perform. (INFINITY: until no improvement is found)
 * @param tabuList A pointer to the tabu list.
 */
void two_opt_tabu(const double *distance_matrix, int nnodes, int *path, double *tour_length, double iterations, CircularBuffer *tabuList);
#endif