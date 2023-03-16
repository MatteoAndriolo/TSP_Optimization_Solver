#ifndef UTILS_H
#define UTILS_H
#include <immintrin.h>
#include <math.h>
#include "logger.h"

/**
 * Calculates the Euclidean distance between two points in 2D space.
 *
 * @param x1 The x-coordinate of the first point.
 * @param y1 The y-coordinate of the first point.
 * @param x2 The x-coordinate of the second point.
 * @param y2 The y-coordinate of the second point.
 * @return The Euclidean distance between the two points.
 */
inline double distance_euclidean(const double *x1, const double *y1, const double *x2, const double *y2);

/**
 * Calculates the squared Euclidean distance between two points in 2D space.
 *
 * @param x1 The x-coordinate of the first point.
 * @param y1 The y-coordinate of the first point.
 * @param x2 The x-coordinate of the second point.
 * @param y2 The y-coordinate of the second point.
 * @return The squared Euclidean distance between the two points.
 */
inline double distance_euclidean_square(const double *x1, const double *y1, const double *x2, const double *y2);

/**
 * Generates a distance matrix for a set of nodes based on their x-y coordinates.
 *
 * @param matrix A pointer to an array that will store the distance matrix.
 * @param nnodes The number of nodes in the graph.
 * @param x An array of length nnodes containing the x-coordinates of the nodes.
 * @param y An array of length nnodes containing the y-coordinates of the nodes.
 * @param round A flag indicating whether to round the distances to the nearest integer.
 */
void generate_distance_matrix(double *matrix, const int nnodes, const double *x, const double *y, int round);

/**
 * Swaps two elements in an integer array.
 *
 * @param arr The array containing the elements to swap.
 * @param i The index of the first element to swap.
 * @param j The index of the second element to swap.
 */
inline void swap(int *arr, int i, int j);

#endif