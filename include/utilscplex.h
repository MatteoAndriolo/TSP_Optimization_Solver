#ifndef UTILS_H
#define UTILS_H
#include <immintrin.h>
#include <math.h>
#include "logger.h"

/**
 * @brief a set of random starting nodes.
 * @param err The error message to print.
 */
void print_error(const char *err);
/**
 * @brief the starting node with the first node in the path
 * @param i The index of the node to start the path at.
 * @param j The index of the node to start the path at.
 * @param inst The instance of the problem to be solved.
 */
int xpos(int i, int j, Instance *inst);
/**
 * @brief the starting node with the first node in the path
 * @param i The index of the node to start the path at.
 * @param j The index of the node to start the path at.
 * @param inst The instance of the problem to be solved.
 */
double dist(int i, int j, Instance *inst);

void xstarToPath(Instance *inst, double *xstar, int dim_xstar, int *path);
#endif