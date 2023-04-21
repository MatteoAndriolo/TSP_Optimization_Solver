#ifndef CONSTRAINT_H
#define CONSTRAINT_H
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include<time.h>
#include<cplex.h>
#include "vrp.h"
#include "utilscplex.h"
#include "logger.h"
#include "tspcplex.h"

/**
 * @brief Add the degree constraint to the model
 * @param inst Instance of the problem
 * @param env CPLEX environment
 * @param lp CPLEX model
*/
void add_degree_constraint(Instance *inst, CPXENVptr env, CPXLPptr lp);

/**
 * @brief Add the capacity constraint to the model
 * @param inst Instance of the problem
 * @param env CPLEX environment
 * @param lp CPLEX model
*/
void add_subtour_constraints(Instance *inst, CPXENVptr env, CPXLPptr lp);

#endif