#ifndef HEURISTICS_H
#define HEURISTICS_h
#include "vrp.h"
#include "plot.h"
#include "refinement.h"
#include "utils.h"
#include "logger.h"

/**
 * Individual structure used for single specimen in the genetic algorithm
 */
typedef struct
{
    int *path;
    double fitness;
} Individual;

/** Nearest Neighbor Heuristic
 * @param distance_matrix: distance matrix
 * @param path: path to be filled
 * @param nnodes: number of nodes
 * @param tour_length: tour length
 */

void vnp_k(const double *distance_matrix, int *path, int nnodes, double *tour_length, int k, int duration);

/** Kick function
 * @param distance_matrix: distance matrix
 * @param path: path to be filled
 * @param nnodes: number of nodes
 * @param tour_length: tour length
 * @param k: number of blocks
 */
void kick_function(const double *distance_matrix, int *path, int nnodes, double *tour_length, int k);

/**
 * Tabu Search
 *
 * @param distance_matrix: distance matrix
 * @param path: path to be filled
 * @param nnodes: number of nodes
 * @param tour_length: tour length
 * @param maxTabuSize: maximum size of tabu list
 */
void tabu_search(const double *distance_matrix, int *path, int nnodes, double *tour_length, int maxTabuSize);

/**
 * Genetic Algorithm
 *
 * @param distance_matrix: distance matrix
 * @param path: path to be filled
 * @param nnodes: number of nodes
 * @param tour_length: tour length
 * @param populationSize: size of the population
 * @param iterations: number of iterations
 */
void genetic_algorithm(const double *distance_matrix, int *path, int nnodes, double *tour_length, int populationSize, int iterations);

// /**
//  * Crossover function
//  *
//  * @param child: child to be filled
//  * @param p1: parent 1
//  * @param p2: parent 2
//  * @param distance_matrix: distance matrix
//  * @param nnodes: number of nodes
//  */
// static int genetic_merge_parents(Individual *child, const Individual *p1, const Individual *p2, const int nnodes, int prob_deletion);

// /**
//  * Repair path child
//  *
//  * @param child: child to be fixed
//  * @param distance_matrix: distance matrix
//  * @param nnodes: number of nodes
//  * @param effort: effort of the repair - timer or iterations
//  */
// void genetic_repair_child(Individual *child, const double *distance_matrix, int nnodes, int effort);

#endif