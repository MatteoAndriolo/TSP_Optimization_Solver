#ifndef TABU_H
#define TABU_H
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "logger.h"
#include "refinement.h"
#include "utils.h"

/**
 * Check if value x is in the buffer
 *
 * @param buf: buffer
 * @param x: value to be checked
 * @return true if x is in the buffer, false otherwise
 */
bool contains(CircularBuffer *buf, int x);

int tabuListContains(int n1, int n2, int tabuList[], int tabuListSize, int maxTabuSize);

/**
 * Tabu Search
 *
 * @param distance_matrix: distance matrix
 * @param path: path to be filled
 * @param nnodes: number of nodes
 * @param tour_length: tour length
 * @param maxTabuSize: maximum size of tabu list
 */
// void tabu_search(const double *distance_matrix, int *path, int nnodes, double *tour_length, int minTabuSize, int maxTabuSize, time_t timelimit);
void tabu_search(const double *distance_matrix, int *path, int nnodes, double *tour_length, int maxTabuSize, time_t timelimit);

/**
 * Test buffer
 */
void test_buffer();

void printBuffer(CircularBuffer *buf);
#endif