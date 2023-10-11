#include "../include/tspcplex.h"
#include "../include/mycallback.h"
#include "../include/refinement.h"
#include "../include/utilscplex.h"

#include <stdbool.h>
#include <string.h>
#define EPS 1e-5

int branch_and_cut(Instance *inst, const CPXENVptr env, const CPXLPptr lp,
                   CPXLONG contextid) {
  if (CPXcallbacksetfunc(env, lp, contextid, my_callback, inst))
    ERROR_COMMENT("tspcplex.c:branch_and_cut", "CPXcallbacksetfunc() error");

  // FROM CHATGPT
  CPXmipopt(env, lp);

  // double *xstar = (double *)calloc(inst->ncols, sizeof(double));
  double xstar[inst->ncols];
  CPXgetx(env, lp, xstar, 0, inst->ncols - 1);

  // int *succ = (int *)calloc(inst->nnodes, sizeof(int));
  // int *comp = (int *)calloc(inst->nnodes, sizeof(int));
  int succ[inst->nnodes];
  int comp[inst->nnodes];
  int ncomp;

  build_sol(xstar, inst, succ, comp, &ncomp);

  double obj_val;
  int error = CPXgetobjval(env, lp, &obj_val);
  if (error)
    ERROR_COMMENT("tspcplex.c:branch_and_cut", "CPXgetobjval() error\n");

  RUN(xstarToPath(inst, xstar, inst->ncols, inst->path));

  RUN(INSTANCE_pathCheckpoint(inst));
  // memcpy(inst->best_path, inst->path, sizeof(int) * inst->nnodes);
  //  free(comp);
  //  free(succ);
  //  free(xstar);

  return 0;
}

int hard_fixing(const CPXENVptr env, const CPXLPptr lp, Instance *inst) {
  DEBUG_COMMENT("tspcplex.c:hard_fixing", "entering hard fixing");
  //----------------------------------------- set the time limit to run cplex
  //---------------------------------------------------------
  // TODO: create a function set the param of cplex
  CPXsetdblparam(env, CPXPARAM_TimeLimit, 15);
  //----------------------------------------- create the xheuristic soltion
  // from
  // a warm start ----------------------------------------
  double *xheu = (double *)calloc(inst->ncols, sizeof(double));
  create_xheu(inst, xheu);
  set_mip_start(inst, env, lp, xheu);

  // double best_solution = INFTY;
  bool no_improv = false;
  while (!no_improv) {
    //----------------------------------------- fixing some edges
    //---------------------------------------------------------------
    fix_edges(env, lp, inst, xheu);
#ifndef PRODUCTION
    CPXwriteprob(env, lp, "mipopt.lp", NULL);
#endif
    //--------------------------------- calling the branch and cut solution
    //-------------------------------------------------------
    inst->solver = 2;
    int status = solve_problem(env, lp,
                               inst); // branch_and_cut(inst, env, lp,
                                      // CPX_CALLBACKCONTEXT_CANDIDATE |
                                      // CPX_CALLBACKCONTEXT_RELAXATION);
    if (status)
      ERROR_COMMENT("tspcplex.c:hard_fixing", "Execution FAILED");
    // ---------------------------------- check if the solution is better
    // than the previous one ---------------------------------
    double actual_solution;
    int error = CPXgetobjval(env, lp, &actual_solution);
    if (error)
      ERROR_COMMENT("tspcplex.c:hard_fixing", "CPXgetobjval() error\n");

    if (actual_solution < inst->best_tourlength) { // UPDATE path
      if (CPXgetx(env, lp, xheu, 0, inst->ncols - 1))
        ERROR_COMMENT("tspcplex.c:hard_fixing", "CPXgetx() error");
      inst->best_tourlength = actual_solution;
      xstarToPath(inst, xheu, inst->ncols, inst->path);
      memcpy(inst->best_path, inst->path, sizeof(int) * inst->ncols);
    }
    //----------------------------------------- unfixing  edges
    //---------------------------------------------------------------
    unfix_edges(env, lp, inst, xheu);

    if (actual_solution >= inst->best_tourlength)
      break;
  }

  inst->solver = 2;
  solve_problem(env, lp, inst);
  if (CPXgetx(env, lp, xheu, 0, inst->ncols - 1))
    ERROR_COMMENT("tspcplex.c:hard_fixing", "CPXgetx() error");
  xstarToPath(inst, xheu, inst->ncols, inst->path);

  free(xheu);
  return 0;
}

// TODO remove this duplicated function already in utliscplex
bool checkxstart2(double *xstar, int dim_xstar) {
  for (int i = 0; i < dim_xstar; i++) {
    if (!(xstar[i] > 0 - EPSILON || xstar[i] < 1 + EPSILON)) {
      return false;
    } else if (xstar[i] > 0.5)
      DEBUG_COMMENT("utilscplex.c:checkxstart", "xstar[%d] = %lf", i, xstar[i]);
  }
  return true;
}

int base_cplex(const CPXENVptr env, const CPXLPptr lp, Instance *inst) {
  CPXsetdblparam(env, CPXPARAM_TimeLimit, 10);
  double *xheu = (double *)calloc(inst->ncols, sizeof(double));
  create_xheu(inst, xheu);
  set_mip_start(inst, env, lp, xheu);
  free(xheu);
  xheu = NULL;

  CPXmipopt(env, lp);
  double *xstar = (double *)calloc(inst->ncols, sizeof(double));
  double obj_val;
  CPXgetobjval(env, lp, &obj_val);
  // double objval = CPX_INFBOUND;
  CPXgetx(env, lp, xstar, 0, inst->ncols - 1);
  CPXgetobjval(env, lp, &obj_val);

  int *succ = (int *)malloc(sizeof(int) * inst->nnodes);
  int *comp = (int *)malloc(sizeof(int) * inst->nnodes);
  int ncomp;
  build_sol(xstar, inst, succ, comp, &ncomp);
  xstarToPath(inst, xstar, inst->ncols, inst->path);

  DEBUG_COMMENT("tspcplex.c:base_cplex", "CPXgetobjval() ");
  if (obj_val < inst->best_tourlength) {
    inst->best_tourlength = obj_val;
    memcpy(inst->best_path, inst->path, sizeof(int) * inst->nnodes);
  }
  DEBUG_COMMENT("tspcplex.c:base_cplex", "END BASE ncomp = %d", ncomp);
  return SUCCESS;
}

int local_branching(const CPXENVptr env, const CPXLPptr lp, Instance *inst) {
  //----------------------------------------- set the time limit to run cplex
  //---------------------------------------------------------
  // TODO: create a function set the param of cplex
  CPXsetdblparam(env, CPXPARAM_TimeLimit, 15);
  //----------------------------------------- create the xheuristic soltion
  // from
  // a warm start ----------------------------------------
  double *xheu = (double *)calloc(inst->ncols, sizeof(double));
  create_xheu(inst, xheu);
  set_mip_start(inst, env, lp, xheu);

  double best_solution = INFTY;
  int no_improv = 0;
  while (no_improv < 1) {
    //------------------------------------------ take current solution and
    // eliminate some solutions -------------------------------
    eliminate_radius_edges(env, lp, inst, xheu, (double)0.90);
#ifndef PRODUCTION
    CPXwriteprob(env, lp, "mipopt.lp", NULL);
#endif
    //--------------------------------- calling the branch and cut solution
    //-------------------------------------------------------
    inst->solver = 2;
    int status = solve_problem(env, lp,
                               inst); // branch_and_cut(inst, env, lp,
                                      // CPX_CALLBACKCONTEXT_CANDIDATE |
                                      // CPX_CALLBACKCONTEXT_RELAXATION);
    if (status)
      ERROR_COMMENT("tspcplex.c:local_branching", "Execution FAILED");
    // ---------------------------------- check if the solution is better
    // than the previous one ---------------------------------
    double actual_solution;
    int error = CPXgetobjval(env, lp, &actual_solution);
    if (error)
      ERROR_COMMENT("tspcplex.c:local_branching", "CPXgetobjval() error\n");

    if (actual_solution < best_solution) { // UPDATE path
      if (CPXgetx(env, lp, xheu, 0, inst->ncols - 1))
        ERROR_COMMENT("tspcplex.c:local_branching", "CPXgetx() error");
      best_solution = actual_solution;
      xstarToPath(inst, xheu, inst->ncols, inst->path);
    }
    //---------------------------------- Unfix the edges and calling
    // thesolution
    //--------------------------------------------------
    repristinate_radius_edges(
        env, lp, inst,
        xheu); // FIXME remove fixing the boundary and add a contraint row
    if (actual_solution >= best_solution)
      break;
  }
  //--------------------------------- calling the branch and cut solution
  //-------------------------------------------------------
  inst->solver = 2;
  solve_problem(env, lp, inst);
  if (CPXgetx(env, lp, xheu, 0, inst->ncols - 1))
    ERROR_COMMENT("tspcplex.c:local_branching", "CPXgetx() error");
  xstarToPath(inst, xheu, inst->ncols, inst->path);
  free(xheu);

  return 0;
}

int solve_problem(const CPXENVptr env, const CPXLPptr lp, Instance *inst) {
  DEBUG_COMMENT("tspcplex.c:solve_problem", "entering solve problem");
  int status = SUCCESS;

  // if (inst->solver == SOLVER_BASE) {
  if (inst->solver == SOLVER_BASE) {
    status = base_cplex(env, lp, inst);
  } else if (inst->solver == SOLVER_BENDER) {
    status = bender(env, lp, inst, false);
  } else if (inst->solver == SOLVER_BRANCH_AND_CUT) {
    status = branch_and_cut(inst, env, lp, -1);
  } else if (inst->solver == SOLVER_PATCHING_HEURISTIC) {
    status = bender(env, lp, inst, true);
  } else if (inst->solver == SOLVER_POSTINGHEU_UCUTFRACT) {
    status = branch_and_cut(inst, env, lp, CPX_CALLBACKCONTEXT_CANDIDATE);
  } else if (inst->solver == SOLVER_MH_HARDFIX) {
    status = hard_fixing(env, lp, inst);
  } else if (inst->solver == SOLVER_MH_LOCBRANCH) {
    status = local_branching(env, lp, inst);
  } else {
    ERROR_COMMENT("tspcplex.c:solve_problem", "Invalid solver selected");
  }

  if (status != SUCCESS) {
    if (status == ERROR_TIME_LIMIT) {
      RUN(INSTANCE_pathCheckpoint(inst));
      INFO_COMMENT("tspcplex.c:solve_problem", "Time limit reached");
    } else {
      ERROR_COMMENT("tspcplex.c:solve_problem", "Execution FAILED");
    }
  }

  return status;
}

void TSPopt(Instance *inst) {
  INFO_COMMENT("tspcplex.c:TSPopt", "Solving TSP with CPLEX");
  int error;
  CPXENVptr env = CPXopenCPLEX(&error);
  if (error)
    ERROR_COMMENT("tspcplex.c:TSPopt", "CPXopenCPLEX() error");
  CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
  if (error)
    ERROR_COMMENT("tspcplex.c:TSPopt", "CPXcreateprob() error");
  inst->env = env;
  inst->lp = lp;

  //------------------------------------------ Building the
  // model----------------------------------------------------------------
  build_model(inst, env, lp);
  INFO_COMMENT("tspcplex.c:TSPopt", "Model built FINISHED");
  CPXwriteprob(env, lp, "model_00.lp", NULL);
  //----------------------------------------- Cplex's parameter setting
  //---------------------------------------------------------
  CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
  CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 1234);
  CPXsetdblparam(env, CPX_PARAM_TILIM,
                 10); // TODO fix 10 with inst->timelimit

  int status = solve_problem(env, lp, inst);
  INSTANCE_pathCheckpoint(inst);
  if (status)
    ERROR_COMMENT("tspcplex.c:TSPopt", "Execution FAILED");

  //------------------------ free and close cplex model
  //--------------------------------------------------------------
  CPXfreeprob(env, &lp);
  CPXcloseCPLEX(&env);
  DEBUG_COMMENT("tspcplex.c:TSPopt", "TSPopt ended");
}

int bender(const CPXENVptr env, const CPXLPptr lp, Instance *inst,
           bool patching) {
  INFO_COMMENT(
      "tspcplex.c:bender",
      "Adding subtour constraints, starting the while loop for BENDERS");
  int ncomp = 0;
  int n_STC = 0;
  int succ[inst->nnodes];
  int comp[inst->nnodes];
  double xstar[inst->ncols];
  double obj_val;
  inst->tend = time(NULL) + inst->max_time;
  // take the time and if exceed the time limit then break the loop
  // time_t start_time = time(NULL);

  // // Set MIP start
  // CPXsetdblparam(env, CPXPARAM_TimeLimit, 10);
  // double *xheu = (double *)calloc(inst->ncols, sizeof(double));
  // create_xheu(inst, xheu);
  // set_mip_start(inst, env, lp, xheu);
  // free(xheu);
  // xheu = NULL;

  DEBUG_COMMENT("tspcplex.c:bender", "initialization terminated");
  int iteration = 0;
  while (ncomp != 1) { // && difftime(time(NULL), start_time) < 10) {
    CPXsetdblparam(env, CPXPARAM_TimeLimit, 10);
    // base_cplex(env, lp, *inst);

    RUN(CPXmipopt(env, lp));

    RUN(CPXgetx(env, lp, xstar, 0, inst->ncols - 1));
    RUN(CPXgetobjval(env, lp, &obj_val));

    build_sol(xstar, inst, succ, comp, &ncomp);
    DEBUG_COMMENT("tspcplex.c:bender", "ncomp = %d", ncomp);

    if (ncomp == 1)
      continue;

    DEBUG_COMMENT("tspcplex.c:bender", "addSubtourConstraints");
    addSubtourConstraints(env, lp, inst->nnodes, comp, inst, &n_STC, xstar);
    char name[30];
    sprintf(name, "benders_%d.lp", iteration);
    iteration++;
    CPXwriteprob(env, lp, name, NULL);

    if (patching) {
      RUN(patchPath(inst, xstar, succ, comp, inst->path, &obj_val));

      RUN(two_opt(inst, 10 * inst->nnodes));
      obj_val = INSTANCE_calculateTourLength(inst);
      if (obj_val < inst->best_tourlength) {
        inst->best_tourlength = obj_val;
        memcpy(inst->best_path, inst->path, inst->nnodes * sizeof(int));
        INFO_COMMENT("tspcplex.c:bender", "found new best obj_value = %lf, ",
                     obj_val);
      }
    }
    // getchar();
    INFO_COMMENT(
        "tspcplex.c:bender",
        "checking time -------------------------------------------------");
    if (checkTime(inst, false) == ERROR_TIME_LIMIT)
      break;
  }

  xstarToPath(inst, xstar, inst->ncols, inst->path);
  RUN(INSTANCE_pathCheckpoint(inst));
  // obj_val = INSTANCE_calculateTourLength(inst);
  // if (obj_val < inst->best_tourlength) {
  //   inst->best_tourlength = obj_val;
  //   memcpy(inst->best_path, inst->path, inst->nnodes * sizeof(int));
  //   DEBUG_COMMENT("tspcplex.c:bender", "found new best obj_value = %lf, ",
  //                 obj_val);
  // }
  // free(xstar);
  // free(succ);
  // free(comp);
  return 0;
}
