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

/**
 * @brief the starting node with the first node in the path
 * @param inst The instance of the problem to be solved.
 * @param xstar solution vector
 * @param dim_xstar dimension of the solution vector
 * @param path solution vector
 */
void xstarToPath(Instance *inst, double *xstar, int dim_xstar, int *path);

/**
 * @brief a set of random starting nodes.
 * @param inst The instance of the problem to be solved.
 */
void create_xheu(Instance *inst, double *xheu, int *path);

/**
 * @brief generate the mip start for the CPLEX problem
 * @param inst the instance of the problem to be solved
 * @param ind
 * @param env CPLEX environment
 * @param lp CPLEX problem
 */
void generate_mip_start(Instance *inst, CPXENVptr env, CPXLPptr lp, double *xheu);
#endif