#ifndef TSPCPLEX_H
#define TSPCPLEX_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <cplex.h>
#include "vrp.h"
#include "logger.h"
#include "constraint.h"
#include "utilscplex.h"
#include "heuristics.h"
#include <concorde.h>

typedef struct
{
    Instance *inst;
    CPXCALLBACKCONTEXTptr context;
} Input;

/**
 * @brief TSPopt
 * @param inst instance of the problem to be solved
 * @param path solution vector
 */
void TSPopt(Instance *inst, int *path);

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
void build_sol(const double *xstar, Instance *inst, int *succ, int *comp, int *ncomp);

/**
 * @brief doit_fn_concorde
 *
 * @param cutval : value of the cut
 * @param cutcount : number of nodes in the cut
 * @param cut : array of members of cut
 * @param inparam : pass_param of CCcut_violated_cuts
 */
int doit_fn_concorde(double cutval, int cutcount, int *cut, void *inparam);

/**
 * @brief my_callback
 *
 * @param context : context of the callback
 * @param contextid : id of the context
 * @param userhandle : userhandle
 */
int my_callback_relaxation(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle);

/**
 * @brief my_callback_candidate
 * @param context : context of the callback
 * @param contextid : id of the context
 * @param userhandle : userhandle
 */
int my_callback_candidate(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle);
/**
 * @brief my_callback
 * @param context : context of the callback
 * @param contextid : id of the context
 * @param userhandle : userhandle
 */
int CPXPUBLIC my_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle);

/**
 * @brief branch_and_cut
 * @param inst instance of the problem to be solved
 * @param env CPLEX environment
 * @param lp CPLEX problem
 * @param contextid id of the context
 */
int branch_and_cut(Instance *inst, CPXENVptr env, CPXLPptr lp, CPXLONG contextid);

/**
 * @brief solve_problem
 * @param env CPLEX environment
 * @param lp CPLEX problem
 * @param inst instance of the problem to be solved
 * @param path solution vector
 */
int solve_problem(CPXENVptr env, CPXLPptr lp, Instance *inst, int *path);

/**
 * @brief hard_fixing
 * @param inst instance of the problem to be solved
 * @param env CPLEX environment
 * @param lp CPLEX problem
 * @param path solution vector
 */
int hard_fixing(CPXENVptr env, CPXLPptr lp, Instance *inst, int *path);

#endif