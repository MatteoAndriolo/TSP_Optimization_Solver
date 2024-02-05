#include "../include/tspcplex.h"

#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "../include/mycallback.h"
#include "../include/refinement.h"
#include "../include/utilscplex.h"
#define EPS 1e-5

double getDuration(Instance *inst) {
  return (double)(clock() - inst->tstart) / CLOCKS_PER_SEC;
}

double getRemainingTime(Instance *inst) {
  return inst->max_time - getDuration(inst);
}

int patch_heuristic(Instance *inst, double *xstar, int *succ, int *comp,
                    int *path, double *obj_val, int *ncomp) {
  RUN(patchPath(inst, xstar, succ, comp, inst->path, obj_val, ncomp));
  RUN(two_opt(inst, INFINITY));
  INFO_COMMENT("tspcplex.c:patch_heuristic", "patch tour lenght = %lf\n",
               inst->tour_length);
  return SUCCESS;
}

int bender(const CPXENVptr env, const CPXLPptr lp, Instance *inst,
           bool patching) {
  CPXsetdblparam(env, CPXPARAM_TimeLimit, getRemainingTime(inst));
  INFO_COMMENT(
      "tspcplex.c:bender",
      "Adding subtour constraints, starting the while loop for BENDERS");
  int ncomp = 0;
  int n_STC = 0;
  int succ[inst->nnodes];
  int comp[inst->nnodes];
  double xstar[inst->ncols];
  double obj_val;
  DEBUG_COMMENT("tspcplex.c:bender", "initialization terminated");
  int iteration = 0;
  double lb = -INFINITY, ub = INFINITY;

  double precision = 0.95;
  bool is1comp = false;
  while (lb < precision * ub) {
    RUN(CPXmipopt(env, lp));
    RUN(CPXgetobjval(env, lp, &obj_val));
    lb = (obj_val > lb) ? obj_val : lb;
    RUN(CPXgetx(env, lp, xstar, 0, inst->ncols - 1));
    build_sol(xstar, inst, succ, comp, &ncomp);
    DEBUG_COMMENT("tspcplex.c:bender", "ncomp = %d", ncomp);
    DEBUG_COMMENT("tspcplex.c:bender", "addSubtourConstraints");
    printf("------------------------------------\n");
    printf(
        "MIPOPT obj_val = %lf,ncomp = %d,lb = %lf, ub = %lf, inst->tl= %lf\n",
        obj_val, ncomp, lb, ub, inst->best_tourlength);
    // xstarToPath(inst, xstar, inst->ncols, inst->path);
    // printf("STORED obj_val = %lf,lb = %lf, ub = %lf, inst->tl= %lf\n",
    //          obj_val, lb, ub, inst->best_tourlength);

    if (ncomp > 1) {
      addSubtourConstraints(env, lp, inst->nnodes, comp, inst, &n_STC, xstar);
      char name[30];
      sprintf(name, "benders_%d.lp", iteration);
      iteration++;
      CPXwriteprob(env, lp, name, NULL);

      if (patching == true) {
        patch_heuristic(inst, xstar, succ, comp, inst->path, &obj_val, &ncomp);
        INSTANCE_calculateTourLength(inst);
        RUN(INSTANCE_pathCheckpoint(inst));
        ub = inst->best_tourlength;
      }
    } else {
      xstarToPath(inst, xstar, inst->ncols, inst->path);
      is1comp = true;
      RUN(INSTANCE_pathCheckpoint(inst));
      ub = inst->best_tourlength;
    }
    // INSTANCE_storeCost(inst,iter);
    //    if (lb >= precision * ub)
    //      DEBUG_COMMENT("tspcplex.c:bender", "Exiting because of lb ub");

    if (is1comp) {
      break;
    }
    printf(
        "END    obj_val = %lf,ncomp = %d,lb = %lf, ub = %lf, inst->tl= %lf\n",
        obj_val, ncomp, lb, ub, inst->best_tourlength);
    printf("------------------------------------\n");
    // ub = inst->best_tourlength;
    CHECKTIME(inst, false);
  }
  CRITICAL_COMMENT("tspcplex.c:bender", "------------------------------------");
  // xstarToPath(inst, xstar, inst->ncols, inst->path);
  // RUN(INSTANCE_pathCheckpoint(inst));
  // INSTANCE_storeCost(inst,iter);
  return 0;
}

int branch_and_cut(Instance *inst, const CPXENVptr env, const CPXLPptr lp,
                   CPXLONG contextid) {
  if (contextid != -1 &&
      CPXcallbacksetfunc(env, lp, contextid, my_callback, inst))
    ERROR_COMMENT("tspcplex.c:branch_and_cut", "CPXcallbacksetfunc() error");

  CPXmipopt(env, lp);

  double xstar[inst->ncols];
  CPXgetx(env, lp, xstar, 0, inst->ncols - 1);

  int *succ = (int *)calloc(inst->ncols, sizeof(int));
  int *comp = (int *)calloc(inst->ncols, sizeof(int));
  int ncomp;

  build_sol(xstar, inst, succ, comp, &ncomp);

  RUN(xstarToPath(inst, xstar, inst->ncols, inst->path));
  RUN(INSTANCE_pathCheckpoint(inst));
  double obj_val;
  int error = CPXgetobjval(env, lp, &obj_val);
  if (error)
    ERROR_COMMENT("tspcplex.c:branch_and_cut", "CPXgetobjval() error\n");
  printf(
      "finished the run of branch and cut with value = %lf,tl=%lf, ncomp= %d\n",
      obj_val, inst->tour_length, ncomp);
  // INSTANCE_storeCost(inst,iter);
  return 0;
}

int hard_fixing(const CPXENVptr env, const CPXLPptr lp, Instance *inst) {
  DEBUG_COMMENT("tspcplex.c:hard_fixing", "entering hard fixing");

  // TODO: create a function set the param of cplex
  // CPXsetdblparam(env, CPXPARAM_TimeLimit, inst->max_time);

  inst->best_tourlength = INFTY;
  double *xheu = (double *)calloc(inst->ncols, sizeof(double));

  // TODO: cerca dove aggiorna la path! in best_path e cambia create xheu da
  // best a normale

  two_opt(inst, inst->ncols);
  INSTANCE_pathCheckpoint(inst);
  // INSTANCE_storeCost(inst,iter);
  create_xheu(inst, xheu);
  set_mip_start(inst, env, lp, xheu);

  int iterations = 5;
  while (iterations-- > 0) {
    double previous_solution_found;
    previous_solution_found = inst->best_tourlength;

    fix_edges(env, lp, inst, xheu);

#ifndef PRODUCTION
    char modelname[30];
    sprintf(modelname, "mipopt_%d.lp", iterations);
    CPXwriteprob(env, lp, modelname, NULL);
#endif

    inst->solver = SOLVER_POSTINGHEU_UCUTFRACT;
    int status = solve_problem(env, lp, inst);
    if (status) ERROR_COMMENT("tspcplex.c:hard_fixing", "Execution FAILED");

    unfix_edges(env, lp, inst, xheu);

    if (previous_solution_found <= inst->best_tourlength) break;
  }
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
  CPXsetdblparam(env, CPXPARAM_TimeLimit, getRemainingTime(inst));
  double *xheu = (double *)calloc(inst->ncols, sizeof(double));
  create_xheu(inst, xheu);
  set_mip_start(inst, env, lp, xheu);
  free(xheu);
  xheu = NULL;

  RUN(INSTANCE_pathCheckpoint(inst));
  CPXmipopt(env, lp);
  double *xstar = (double *)calloc(inst->ncols, sizeof(double));
  double obj_val;
  CPXgetobjval(env, lp, &obj_val);
  // double objval = CPX_INFBOUND;
  CPXgetx(env, lp, xstar, 0, inst->ncols - 1);
  // CPXgetobjval(env, lp, &obj_val);

  int *succ = (int *)malloc(sizeof(int) * inst->nnodes);
  int *comp = (int *)malloc(sizeof(int) * inst->nnodes);
  int ncomp;
  build_sol(xstar, inst, succ, comp, &ncomp);
  xstarToPath(inst, xstar, inst->ncols, inst->path);
  printf("------------------------------------\n");
  printf("ncomp = %d\n", ncomp);
  printf("------------------------------------\n");

  patch_heuristic(inst, xstar, succ, comp, inst->path, &obj_val, &ncomp);
  INSTANCE_pathCheckpoint(inst);
  obj_val = inst->best_tourlength;

  printf("------------------------------------\n");
  printf("ncomp = %d\n", ncomp);
  printf("obj_val = %lf\n", obj_val);
  printf("------------------------------------\n");

  DEBUG_COMMENT("tspcplex.c:base_cplex", "CPXgetobjval() ");
  if (obj_val < inst->best_tourlength) {
    inst->best_tourlength = obj_val;
    memcpy(inst->best_path, inst->path, sizeof(int) * inst->nnodes);
  }
  RUN(INSTANCE_pathCheckpoint(inst));
  DEBUG_COMMENT("tspcplex.c:base_cplex", "END BASE ncomp = %d", ncomp);
  return SUCCESS;
}

int local_branching(const CPXENVptr env, const CPXLPptr lp, Instance *inst) {
  inst->best_tourlength = INFTY;
  // TODO: create a function set the param of cplex

  double *xheu = (double *)calloc(inst->ncols, sizeof(double));

  two_opt(inst, inst->ncols);
  INSTANCE_pathCheckpoint(inst);
  // INSTANCE_storeCost(inst,iter);
  create_xheu(inst, xheu);
  set_mip_start(inst, env, lp, xheu);
  double previous_solution_found = INFTY;
  inst->best_tourlength = INFTY;

#ifndef PRODUCTION
  int iterations = 0;
#endif
  int iteration = 1;

  int *indexes = (int *)malloc(inst->ncols * sizeof(int));
  double *values = (double *)malloc(inst->ncols * sizeof(double));

  while (iteration++ < 6) {
    CPXsetdblparam(env, CPXPARAM_TimeLimit, 200);

    previous_solution_found = inst->best_tourlength;

    // eliminate_radius_edges(env, lp, inst, xheu, inst->percentageLB);

    int nrows = CPXgetnumrows(env, lp);

    // Add new constraints according to the radius: SUM_{x_e=1}{x_e}>=n-radius
    int nnz = 0;
    for (int i = 0; i < inst->ncols; i++) {
      if (xheu[i] > 0.5) {
        indexes[nnz] = i;
        values[nnz] = 1.0;
        nnz++;
      }
    }
    // double rhs = inst->nnodes - K;
    // in case change all the edges
    double rhs = inst->nnodes - (int)(inst->nnodes * inst->percentageLB);
    char **cname = malloc(sizeof(char *));
    cname[0] = malloc(30 * sizeof(char));
    sprintf(cname[0], "subtour_LB(%d)", iteration);
    char sense = 'L';
    int rmatbeg[] = {0};
    int status = CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, rmatbeg, indexes,
                            values, NULL, cname);

    if (status) printf("CPXaddrows() error");

#ifndef PRODUCTION
    char modelname[30];
    sprintf(modelname, "mipopt_%d.lp", iterations);
    CPXwriteprob(env, lp, modelname, NULL);
#endif

    inst->solver = SOLVER_POSTINGHEU_UCUTFRACT;

    status = solve_problem(env, lp, inst);

    if (status) ERROR_COMMENT("tspcplex.c:local_branching", "Execution FAILED");

#ifndef PRODUCTION
    sprintf(modelname, "mipopt_%d_solver.lp", iterations);
    CPXwriteprob(env, lp, modelname, NULL);
#endif

    if (CPXdelrows(env, lp, nrows, nrows)) printf("CPXdelrows() error");

      // repristinate_radius_edges(env, lp, inst, xheu);

#ifndef PRODUCTION
    sprintf(modelname, "mipopt_%d_repri.lp", iterations);
    CPXwriteprob(env, lp, modelname, NULL);
#endif

    if (previous_solution_found <= inst->best_tourlength) {
      printf("previous_solution_found = %lf\n", previous_solution_found);
      // break;
    }
    create_xheu(inst, xheu);
    CHECKTIME(inst, false);
  }

  xstarToPath(inst, xheu, inst->ncols, inst->path);
  free(xheu);
  return 0;
}

ErrorCode solve_problem(const CPXENVptr env, const CPXLPptr lp,
                        Instance *inst) {
  DEBUG_COMMENT("tspcplex.c:solve_problem", "entering solve problem");
  ErrorCode status = SUCCESS;

  // if (inst->solver == SOLVER_BASE) {
  if (inst->solver == SOLVER_BASE) {  // cplex
    DEBUG_COMMENT("tspcplex.c:solve_problem", "SOLVER_BASE");
    status = base_cplex(env, lp, inst);
  } else if (inst->solver == SOLVER_BENDER) {  // cplexbend
    DEBUG_COMMENT("tspcplex.c:solve_problem", "SOLVER_BENDER");
    status = bender(env, lp, inst, false);
  } else if (inst->solver == SOLVER_PATCHING_HEURISTIC) {  // cplexpatch
    DEBUG_COMMENT("tspcplex.c:solve_problem", "SOLVER_PATCHING_HEURISTIC");
    status = bender(env, lp, inst, true);
  } else if (inst->solver == SOLVER_BRANCH_AND_CUT) {  // cplexbc
    DEBUG_COMMENT("tspcplex.c:solve_problem", "SOLVER_BRANCH_AND_CUT");
    status = branch_and_cut(inst, env, lp, CPX_CALLBACKCONTEXT_CANDIDATE);
  } else if (inst->solver == SOLVER_POSTINGHEU_UCUTFRACT) {  // cplexpost
    DEBUG_COMMENT("tspcplex.c:solve_problem", "SOLVER_POSTINGHEU_UCUTFRACT");
    status = branch_and_cut(
        inst, env, lp,
        CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_RELAXATION);

  } else if (inst->solver == SOLVER_MH_HARDFIX) {  // cplexhf
    DEBUG_COMMENT("tspcplex.c:solve_problem", "SOLVER_MH_HARDFIX");
    status = hard_fixing(env, lp, inst);
  } else if (inst->solver == SOLVER_MH_LOCBRANCH) {  // cplexlb
    DEBUG_COMMENT("tspcplex.c:solve_problem", "SOLVER_MH_LOCBRANCH");
    status = local_branching(env, lp, inst);
  } else {
    ERROR_COMMENT("tspcplex.c:solve_problem", "Invalid solver selected");
  }

  if (status != SUCCESS) {
    if (status == ERROR_TIME_LIMIT) {
      // RUN(INSTANCE_pathCheckpoint(inst));
      // INSTANCE_storeCost(inst,iter);
      WARNING_COMMENT("tspcplex.c:solve_problem", "Time limit reached");
    } else {
      ERROR_COMMENT("tspcplex.c:solve_problem", "Execution FAILED");
    }
  }

  return status;
}

ErrorCode TSPopt(Instance *inst) {
  INFO_COMMENT("tspcplex.c:TSPopt", "Solving TSP with CPLEX");
  int error;
  CPXENVptr env = CPXopenCPLEX(&error);
  if (error) ERROR_COMMENT("tspcplex.c:TSPopt", "CPXopenCPLEX() error");
  CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
  if (error) ERROR_COMMENT("tspcplex.c:TSPopt", "CPXcreateprob() error");
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
  CPXsetdblparam(env, CPX_PARAM_TILIM, inst->max_time);
  // CPXsetintparam(env, CPX_PARAM_THREADS, 1);
  //  CPXsetdblparam(env, CPX_PARAM_TILIM, time(NULL) - inst->max_time);
  //   CPXsetdblparam(env, CPX_PARAM_TILIM, 100);

  RUN(solve_problem(env, lp, inst));
  // INSTANCE_storeCost(inst,iter);

  //------------------------ free and close cplex model
  //--------------------------------------------------------------
  CPXfreeprob(env, &lp);
  CPXcloseCPLEX(&env);
  DEBUG_COMMENT("tspcplex.c:TSPopt", "TSPopt ended");
  return SUCCESS;
}
