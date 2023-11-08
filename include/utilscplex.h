#ifndef UTILS_H
#define UTILS_H
#include <math.h>

#include "errors.h"
#include "vrp.h"
// #include <cplex.h>

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
 * @return ErrorCode
 */
ErrorCode xstarToPath(Instance *inst, const double *xstar, int dim_xstar,
                      int *path);

/**
 * @brief a set of random starting nodes.
 * @param inst The instance of the problem to be solved.
 */
void create_xheu(Instance *inst, double *xheu);

/**
 * @brief generate the mip start for the CPLEX problem
 * @param inst the instance of the problem to be solved
 * @param ind
 * @param env CPLEX environment
 * @param lp CPLEX problem
 */
void set_mip_start(Instance *inst, CPXENVptr env, CPXLPptr lp, double *xheu);
/**
 * @brief generate the fix edfges set for hard fixing
 * @param env CPLEX environment
 * @param lp CPLX problem
 * @param inst the instance of the problem to be solved
 * @param xheu the solution vector
 */
void fix_edges(CPXENVptr env, CPXLPptr lp, Instance *inst, double *xheu);
/**
 * @brief generate the unfix edfges set for hard fixing
 * @param env CPLEX environment
 * @param lp CPLX problem
 * @param inst the instance of the problem to be solved
 * @param xheu the solution vector
 */
void unfix_edges(CPXENVptr env, CPXLPptr lp, Instance *inst, double *xheu);

/**
 * @brief generate the variables to be eliminate from the current solution
 * @param env CPLEX environment
 * @param lp CPLX problem
 * @param inst the instance of the problem to be solved
 */
void eliminate_radius_edges(CPXENVptr env, CPXLPptr lp, Instance *inst,
                            double *xheu, int radious);

/**
 * @brief generate the variables to be eliminate from the current solution
 * @param env CPLEX environment
 * @param lp CPLX problem
 * @param inst the instance of the problem to be solved
 * @param xheu the solution vector
 */
void repristinate_radius_edges(CPXENVptr env, CPXLPptr lp, Instance *inst,
                               double *xheu);

/**
 * @brief build_sol
 * @param xstar solution vector
 * @param inst instance of the problem to be solved
 * @param succ successor vector
 * @param comp component vector
 * @param ncomp number of components
 */
void build_sol(const double *xstar, Instance *inst, int *succ, int *comp,
               int *ncomp);
void init_mip(const CPXENVptr env, const CPXLPptr lp, Instance *inst);

/*
 * @brief build_model
 * @param inst instance of the problem to be solved
 * @param env CPLEX environment
 * @param lp CPLEX problem
 */
void build_model(Instance *inst, const CPXENVptr env,
                 const CPXLPptr lp); // CPXENVptr env, CPXLPptr lp);
/*
 * @brief build_model
 * @param inst instance of the problem to be solved
 * @param env CPLEX environment
 * @param lp CPLEX problem
 */
void build_model(Instance *inst, const CPXENVptr env,
                 const CPXLPptr lp); // CPXENVptr env, CPXLPptr lp);
int addSubtourConstraints(CPXENVptr env, CPXLPptr lp, int nnodes, int *comp,
                          Instance *inst, int *counter, double *xstar);

int patchPath(Instance *inst, double *xstar, int *succ, int *comp, int *path,
              double *obj_value, int *ncomp);
#endif
