#ifndef TSPCPLEX_H
#define TSPCPLEX_H
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// #include <cplex.h>
#include "../tmpcplex/cplex.h"

#include "vrp.h"

// typedef struct{
//   TSPSolvers solver;
//
//   double percentageHF;
//   int ncols;
//
//   CPXENVptr env;
//   CPXLPptr lp;
//   CPXCALLBACKCONTEXTptr context;
// }CPLEX_MODEL;

typedef struct {
  CPXCALLBACKCONTEXTptr context;
  Instance *inst;
  double *xstar;
} Params_CC;

/**
 * @brief TSPopt
 * @param inst instance of the problem to be solved
 */
void TSPopt(Instance *inst);

/**
 * @brief solve_problem
 * @param env CPLEX environment
 * @param lp CPLEX problem
 * @param inst instance of the problem to be solved
 */
int solve_problem(const CPXENVptr env, const CPXLPptr lp, Instance *inst);

int base_cplex(const CPXENVptr env, const CPXLPptr lp, Instance *inst);

/**
 * @brief branch_and_cut
 * @param inst instance of the problem to be solved
 * @param env CPLEX environment
 * @param lp CPLEX problem
 * @param contextid id of the context
 */
int branch_and_cut(Instance *inst, const CPXENVptr env, const CPXLPptr lp,
                   CPXLONG contextid);

/**
 * @brief hard_fixing
 * @param inst instance of the problem to be solved
 * @param env CPLEX environment
 * @param lp CPLEX problem
 */
int hard_fixing(const CPXENVptr env, const CPXLPptr lp, Instance *inst);

/**
 * @brief local_branching
 * @param inst instance of the problem to be solved
 * @param env CPLEX environment
 * @param lp CPLEX problem
 */
int local_branching(const CPXENVptr env, const CPXLPptr lp, Instance *inst);

/**
 * @brief generate the variables to be eliminate from the current solution
 * @param env CPLEX environment
 * @param lp CPLX problem
 * @param inst the instance of the problem to be solved
 */
int bender(CPXENVptr env, CPXLPptr lp, Instance *inst, bool patching);
#endif
