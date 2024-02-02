#ifndef UTILS_H
#define UTILS_H
#include <immintrin.h>
#include <math.h>
#include <stdbool.h>

#include "vrp.h"

#define FREE(ptr) \
  free(ptr);      \
  ptr = NULL;

/**
 * Swaps two elements in an integer array.
 *
 * @param arr The array containing the elements to swap.
 * @param i The index of the first element to swap.
 * @param j The index of the second element to swap.
 */
void swap(int *arr, int i, int j);

void swapPathPoints(Instance *inst, int i, int j);

void shuffle(int *arr, int n);

/**
 * Swaps two elements in an integer array and shifts all elements between
 * them.
 *
 * @param arr The array containing the elements to swap.
 * @param i The index of the first element to swap.
 * @param j The index of the second element to swap.
 * @param n The length of the array. *
 */

void swap_and_shift(int *arr, int i, int j, int n);

/**
 * Insert new_entry into the ranking array if it is better than the worst
 * entry. The ranking array is sorted in ascending order.
 *
 * @param ranking The ranking array.
 * @param n_ranks The number of entries in the ranking array.
 * @param new_entry The new entry to insert into the ranking array.
 */
void replace_if_better(int *r_index, double *r_value, int n_ranks,
                       int new_index, double new_value);

/**
 * Calculates the Euclidean distance between two points in 2D space.
 *
 * @param x1 The x-coordinate of the first point.
 * @param y1 The y-coordinate of the first point.
 * @param x2 The x-coordinate of the second point.
 * @param y2 The y-coordinate of the second point.
 * @return The Euclidean distance between the two points.
 */
double distance_euclidean(double x1, double y1, double x2, double y2);

/**
 * Calculates the squared Euclidean distance between two points in 2D space.
 *
 * @param x1 The x-coordinate of the first point.
 * @param y1 The y-coordinate of the first point.
 * @param x2 The x-coordinate of the second point.
 * @param y2 The y-coordinate of the second point.
 * @return The squared Euclidean distance between the two points.
 */
double distance_euclidean_square(double x1, double y1, double x2, double y2);

/**
 * Is path feasible?
 *
 * @param path The path to check.
 * @param nnodes The number of nodes in the graph.
 * @return 1 if the path is feasible, 0 otherwise.
 */
int feasiblePath(const int *path, const int nnodes);

/**
 * Asserts that a path is valid.
 *
 * @param path The path to check.
 * @param distance_matrix The distance matrix for the graph.
 * @param nnodes The number of nodes in the graph.
 * @param tour_length The length of the tour.
 */
int assert_path(const int *path, const double *distance_matrix,
                const int nnodes, const double tour_length);

/**
  swap the starting node with the first node in the path
 * @param path The path to check.
 * @param starting_node The index of the node to start the path at.
 * @param num_nodes The number of nodes in the graph.
 */
void set_starting_node(int *path, int starting_node, int num_nodes);

/**
 * Generates a set of random starting nodes.
 *
 * @param starting_nodes A pointer to an array that will store the starting
 * nodes.
 * @param num_instances The number of starting nodes to generate.
 * @param seed The seed for the random number generator.
 * @param num_nodes The number of nodes in the graph.
 */
void generate_random_starting_nodes(int *starting_nodes, int num_nodes,
                                    int num_instances, int seed);

/**
 * Generates a set of random starting nodes.
 * @param arr The array to shuffle.
 * @param start1 starting index 1.
 * @param end1 ending index 1.
 * @param start2 starting index 2.
 * @param end2 ending index 2
 */
void swap_array_piece(int *arr, int start1, int end1, int start2, int end2);

/**
 * Calculates the length of a tour.
 *
 * @param path The path to check.
 * @param distance_matrix The distance matrix for the graph.
 * @return The length of the tour.
 */
double get_tour_length(const int *path, const int nnodes,
                       const double *distance_matrix);

/**
 * Calculates the length of a tour.
 *
 * @param path The path to check.
 * @param i The index of the first node in the tour.
 * @param j The index of the second node in the tour.
 * @param nnodes The number of nodes in the graph.
 * @return The length of the tour.
 */
void two_opt_move(Instance *inst, int n1, int n2);

/**
 * Generate random int
 *
 * @param lowerBound lower bound
 * @param upperBound upper bound
 */
int randomBetween(int lowerBound, int upperBound);

/**
 * Generate random double
 *
 * @param lowerBound lower bound
 * @param upperBound upper bound
 */
double randomBetween_d(double lowerBound, double upperBound);

/**
 * Check path correctness
 *
 * @param path pointer to path
 * @param nnodes The number of nodes in the graph.
 */
bool simpleCorrectness(int *path, int nnodes);

void print_nodes(double *x, double *y, int nnodes);

double generate_random_path(Instance *inst, int *path);

#endif
