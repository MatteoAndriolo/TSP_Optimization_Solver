#ifndef REFINEMENT_H
#define REFINEMENT_H
#include <stdio.h>
#include <stdlib.h>
#include "logger.h"

/**
 * Applies the 2-opt algorithm to a given distance matrix and node list (in place).
 * 
 * @param matrix A pointer to the distance matrix.
 * @param size_node The number of nodes in the tour.
 * @param node A pointer to the array of nodes in the tour.
 *
 * @return void
 */
void two_opt(double *matrix, int size_node, int *path, double *tour_lenght);

#endif