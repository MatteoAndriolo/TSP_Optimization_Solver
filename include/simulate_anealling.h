#ifndef SIMULATE_ANEALLING_h
#define SIMULATE_ANEALLING_h
#include "vrp.h"
#include "plot.h"
#include "refinement.h"
#include "utils.h"
#include "logger.h"
/**
 * @brief Simulate anealling algorithm
 * @param distance_matrix Distance matrix
 * @param path Path
 * @param nnodes Number of nodes
 * @param tour_length Tour length
 * @param k Number of iterations
 */
void simulate_anealling(const double *distance_matrix, int *path, int nnodes, double *tour_length, int k, int duration);
/**
 * @brief Split path
 * @param distance_matrix Distance matrix
 * @param path Path
 * @param nnodes Number of nodes
 * @param tour_length Tour length
 */

void split_path(const double *distance_matrix, int *path, int nnodes, double *tour_length);

#endif
