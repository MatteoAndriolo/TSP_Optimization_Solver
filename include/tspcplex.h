#ifndef TSPCPLEX_H
#define TSPCPLEX_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "../tmpcplex/cplex.h"
//#include "../tmpcplex/cplexx.h"
//#include <cplex.h>
#include <concorde.h>
//#include "../concorde_build/concorde.h"

#include "vrp.h"
#include "logger.h"
#include "utilscplex.h"



//typedef struct{
//  TSPSolvers solver;
//
//  double percentageHF;
//  int ncols;
//
//  CPXENVptr env;
//  CPXLPptr lp;
//  CPXCALLBACKCONTEXTptr context;
//}CPLEX_MODEL;

typedef struct{
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
 * @brief build_model
 * @param inst instance of the problem to be solved
 * @param env CPLEX environment
 * @param lp CPLEX problem
 */
void build_model(Instance *inst,CPXENVptr env, CPXLPptr lp); // CPXENVptr env, CPXLPptr lp);

/**
 * @brief solve_problem
 * @param env CPLEX environment
 * @param lp CPLEX problem
 * @param inst instance of the problem to be solved
 */
int solve_problem(CPXENVptr env, CPXLPptr lp, Instance *inst);

int base_cplex(CPXENVptr env, CPXLPptr lp, Instance *inst);

/**
 * @brief doit_fn_concorde
 *
 * @param cutval : value of the cut
 * @param cutcount : number of nodes in the cut
 * @param cut : array of members of cut
 * @param inparam : pass_param of CCcut_violated_cuts
 */
//int doit_fn_concorde(double cutval, int cutcount, int *cut, void *inparam);

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
//FIXME was written int CPXPUBLIC ??
int my_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle);

/**
 * @brief branch_and_cut
 * @param inst instance of the problem to be solved
 * @param env CPLEX environment
 * @param lp CPLEX problem
 * @param contextid id of the context
 */
int branch_and_cut(Instance *inst, CPXENVptr env, CPXLPptr lp, CPXLONG contextid);


/**
 * @brief hard_fixing
 * @param inst instance of the problem to be solved
 * @param env CPLEX environment
 * @param lp CPLEX problem
 */
int hard_fixing(CPXENVptr env, CPXLPptr lp, Instance *inst);

/**
 * @brief local_branching
 * @param inst instance of the problem to be solved
 * @param env CPLEX environment
 * @param lp CPLEX problem
 */
int local_branching(CPXENVptr env, CPXLPptr lp, Instance *inst);

#endif
