#ifndef TSPCPLEX_H
#define TSPCPLEX_H
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<cplex.h>
#include "vrp.h"
#include "utils.h"
#include "logger.h"

/**
 * @brief TSPopt
 * @param inst instance of the problem to be solved
 * @param path solution vector
*/
void TSPopt(Instance *inst, int* path);
/**
 * @brief build_model
 * @param inst instance of the problem to be solved
 * @param env CPLEX environment
 * @param lp CPLEX problem
*/
void build_model(Instance *inst, CPXENVptr env, CPXLPptr lp);
/**
 * @brief build_sol
 * @param xstar solution vector
 * @param inst instance of the problem to be solved
 * @param succ successor vector
 * @param comp component vector
 * @param ncomp number of components
*/
void build_sol(const double *xstar, Instance *inst, int * succ, int *comp, int* ncomp);

#endif