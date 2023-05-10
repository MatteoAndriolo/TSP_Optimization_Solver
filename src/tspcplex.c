#include "tspcplex.h"
#define EPS 1e-5

void build_model(Instance *inst, CPXENVptr env, CPXLPptr lp)
{
  INFO_COMMENT("tspcplex:build_model", "Building model");
  // double zero = 0.0;
  char binary = 'B';

  char **cname = (char **)calloc(1, sizeof(char *)); // (char **) required by cplex...
  cname[0] = (char *)calloc(100, sizeof(char));

  // add binary var.s x(i,j) for i < j

  for (int i = 0; i < inst->nnodes; i++)
  {
    for (int j = i + 1; j < inst->nnodes; j++)
    {
      sprintf(cname[0], "x(%d,%d)", i + 1, j + 1); // (i+1,j+1) because CPLEX starts from 1 (not 0
      double obj = dist(i, j, inst);               // cost == distance
      double lb = 0.0;
      double ub = 1.0;
      // eviroment , linear programming, cost of distance of x(i,j), lower bound, upper bound, binary, name of x(i,j)
      if (CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname))
        ERROR_COMMENT("tspcplex::build_model", " wrong CPXnewcols on x var.s");
      if (CPXgetnumcols(env, lp) - 1 != xpos(i, j, inst))
        ERROR_COMMENT("tspcplex::build_model", " wrong position for x var.s");
    }
  }
  INFO_COMMENT("tspcplex:build_model", "number of columns in CPLEX %d", CPXgetnumcols(env, lp));

  add_degree_constraint(inst, env, lp); // add degree constraints (for each node
  inst->ncols = CPXgetnumcols(env, lp); // get number of columns (variables) in CPLEX model
  free(cname[0]);
  free(cname);
}

// https://www.math.uwaterloo.ca/tsp/concorde/DOC/cut.html#CCcut_violated_cuts
/**
 * @brief doit_fn_concorde
 *
 * @param cutval : value of the cut
 * @param cutcount : number of nodes in the cut
 * @param cut : array of members of cut
 * @param inparam : pass_param of CCcut_violated_cuts
 */
int doit_fn_concorde(double cutval, int cutcount, int *cut, void *inparam)
{
  Input *in = (Input *)inparam;
  int num_nonzero = 0;
  int ncols = CPXgetnumcols(in->env, in->lp);
  int *cmatbeg = malloc(ncols * sizeof(int));
  int *cmatind = malloc(ncols * sizeof(int));
  double *cmatval = malloc(ncols * sizeof(double));
  int cmatspace = ncols;
  int *surplus_p = 0;
  CPXgetcols(in->env, in->lp, &num_nonzero, cmatbeg, cmatind, cmatval, cmatspace, surplus_p, 0, ncols);
  // https://www.ibm.com/docs/en/cofz/12.8.0?topic=cpxxcallbackaddusercuts-cpxcallbackaddusercuts
  int status = CPXcallbackaddusercuts(in->context, 1, cutcount, (double *)&cutval, (char *)'L', cmatbeg, cmatind, cmatval, (int *)CPX_USECUT_FILTER, 0);
  return status;
  return 0;
}

/*
int  doit_fn_concorde(double cutval, int cutcount, int * cut, void* in_param){
  Input inp= (Input*) in_param;
  UserCutCallback(inp->env, inp->cbdata, inp->wherefrom, inp->useraction_p);
}
*/

int my_callback_relaxation(CPXLPptr lp, CPXENVptr env, CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle, double *xstar)
{
  INFO_COMMENT("tspcplex.c:my_callback_relaxation", "entering relaxation callbacks");
  Instance *inst = (Instance *)userhandle;
  int ecount = inst->nnodes * (inst->nnodes - 1) / 2;
  int ncomp = 0;
  int **comps_count = (int **)malloc(sizeof(int *));
  int **comps = (int **)malloc(sizeof(int *));
  int elist[ecount];
  int loader = 0;
  for (int i = 0; i < inst->nnodes; i++)
  {
    for (int j = i + 1; j < inst->nnodes; j++)
    {
      elist[loader++] = i;
      elist[loader++] = j;
    }
  }
  // https://www.math.uwaterloo.ca/tsp/concorde/DOC/cut.html#CCcut_violated_cuts
  // char *tmp;
  // DEBUG_COMMENT("tspcplex.c:my_callback_relaxation", "before xstar");
  // tmp = getPathDBL(xstar, inst->nnodes*(inst->nnodes-1)/2);
  // DEBUG_COMMENT("tspcplex.c:my_callback_relaxation", "xstar = %s", tmp);
  // free(tmp);
  // DEBUG_COMMENT("tspcplex.c:my_callback_relaxation", "xstar 1 %lf", xstar[0]);
  INFO_COMMENT("tspcplex.c:my_callback_relaxation", "calling CCcut_connect_components");
  if (CCcut_connect_components(inst->nnodes, ecount, elist, xstar, &ncomp, comps_count, comps))
    print_error("CCcut_connect_components error"); // FIXME seg fault inside CCcut_connect_components
  DEBUG_COMMENT("tspcplex.c:my_callback_relaxation", "exit CCcut_connect_components");
  if (ncomp == 1)
  {
    DEBUG_COMMENT("tspcplex.c:my_callback_relaxation", "ncomp = %d", ncomp);
    Input *in = (Input *)malloc(sizeof(Input));
    in->context = context;
    in->elist = elist;
    in->env = env;
    in->inst = inst;
    in->lp = lp;
    // in->useraction_p
    // in->wherefrom

    if (CCcut_violated_cuts(inst->nnodes, ecount, elist, xstar, 2.0 - EPSILON, doit_fn_concorde, (void *)in))
      print_error("CCcut_violated_cuts error");
  }
  return 0;
}

int my_callback_encumbment(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle, double *xstar)
{
  Instance *inst = (Instance *)userhandle;
  // if xstart is infeasible, find a violated cut and store it in the usual Cplex's data structute (rhs, sense, nnz, index and value)
  int *succ = (int *)malloc(sizeof(int) * inst->nnodes);
  int *comp = (int *)malloc(sizeof(int) * inst->nnodes);
  int ncomp;
  build_sol(xstar, inst, succ, comp, &ncomp);
  DEBUG_COMMENT("constraint.c:add_subtour_constraints", "ncomp = %d", ncomp);
  DEBUG_COMMENT("constraint.c:add_subtour_constraint", "succ: %s", getPath(succ, inst->nnodes));
  DEBUG_COMMENT("constraint.c:add_subtour_constraint", "comp: %s", getPath(comp, inst->nnodes));

  if (ncomp > 1)
  {
    for (int cc = 1; cc <= ncomp; cc++)
    {
      int nncc = 0;
      for (int i = 0; i < inst->nnodes; i++)
      {
        if (comp[i] == cc)
          nncc++;
      }

      int *index = (int *)calloc((inst->nnodes * (inst->nnodes - 1)) / 2, sizeof(int));
      double *value = (double *)calloc((inst->nnodes * (inst->nnodes - 1)) / 2, sizeof(double));
      char sense = 'L'; // 'L' for less than or equal to constraint
      int nnz = 0;

      int j = 0;
      int k;
      while (j < inst->nnodes - 1)
      {
        while (comp[j] != cc && j < inst->nnodes)
        {
          j++;
        }
        k = j + 1;
        while (k < inst->nnodes)
        {
          if (comp[k] == cc)
          {
            index[nnz] = xpos(j, k, inst);
            value[nnz] = 1.0;
            nnz++;
          }
          k++;
        }
        j++;
      }

      DEBUG_COMMENT("constraint.c:add_subtour_constraints", "nnz = %d", nnz);
      int izero = 0;
      double rsh = nncc - 1;

      DEBUG_COMMENT("constraint.c:add_subtour_constraints", "insert row");
      if (CPXcallbackrejectcandidate(context, 1, nnz, &rsh, &sense, &izero, index, value))
        ERROR_COMMENT("constraint.c:my_callback", "CPXaddrows(): error 1");
      // free(cname[0]);
      // free(cname);
      free(index);
      free(value);
      WARNING_COMMENT("constraint.c:add_subtour_constraints", "FREEING MEMORY, index, value, index1, cname[0], cname");
    }
  }
  DEBUG_COMMENT("tspcplex.c:my_callback_encumbment", " final ncomp %d ", ncomp);
  free(succ);
  free(comp);
  return 0;
}

static int CPXPUBLIC my_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle)
{
  INFO_COMMENT("tspcplex.c:my_callback", "entering callbacks");
  Instance *inst = (Instance *)userhandle;
  double *xstar = (double *)malloc(inst->ncols * sizeof(double));
  double objval = CPX_INFBOUND;
  inst->context = context;
  // int izero = 0;
  // int purgeable = CPX_USECUT_FILTER;
  // int local = 0;

  if (contextid == CPX_CALLBACKCONTEXT_RELAXATION && CPXcallbackgetrelaxationpoint(context, xstar, 0, inst->ncols - 1, &objval))
    print_error("CPXcallbackgetrelaxationpoint error");
  if (contextid == CPX_CALLBACKCONTEXT_CANDIDATE && CPXcallbackgetcandidatepoint(context, xstar, 0, inst->ncols - 1, &objval))
    print_error("CPXcallbackgetcandidatepoint error");

  if (contextid == CPX_CALLBACKCONTEXT_RELAXATION)
  {
    DEBUG_COMMENT("tspcplex.c:my_callback", "xsta 0 %lf", xstar[1]);
    char *tmp;
    DEBUG_COMMENT("tspcplex.c:my_callback", "before xstar");
    tmp = getPathDBL(xstar, inst->nnodes * (inst->nnodes - 1) / 2);
    DEBUG_COMMENT("tspcplex.c:my_callback", "xstar = %s", tmp);
    free(tmp);

    my_callback_relaxation(inst->lp, inst->env, context, contextid, userhandle, xstar);
  }

  print_error("CPXcallbackgetrelaxationpoint error");

  if (contextid == CPX_CALLBACKCONTEXT_CANDIDATE)
  {
    my_callback_encumbment(context, contextid, userhandle, xstar);
  }

  free(xstar);
  return 0;
}

void TSPopt(Instance *inst, int *path, int callbacks)
{
  INFO_COMMENT("tspcplex.c:TSPopt", "Solving TSP with CPLEX");
  // open CPLEX model
  int error;
  CPXENVptr env = CPXopenCPLEX(&error);
  if (error)
    print_error("CPXopenCPLEX() error");
  CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
  if (error)
    print_error("CPXcreateprob() error");

  // Cplex's parameter setting
  CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
  CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 123456);
  CPXsetdblparam(env, CPX_PARAM_TILIM, 3600.0);

  // Building the model
  build_model(inst, env, lp);
  INFO_COMMENT("tspcplex.c:TSPopt", "Model built FINISHED");

  inst->lp = lp;
  inst->env = env;
  // CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_RELAXATION;
  // CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_RELAXATION;
  CPXLONG contextid = CPX_CALLBACKCONTEXT_RELAXATION;
  if (CPXcallbacksetfunc(env, lp, contextid, my_callback, inst))
    ERROR_COMMENT("tspcplex.c:TSPopt", "CPXcallbacksetfunc() error");
  // add_subtour_constraints(inst, env, lp);
  error = CPXmipopt(env, lp);
  if (error)
  {
    printf("CPX error code %d\n", error);
    print_error("CPXmipopt() error");
  }

  // use the optimal solution found by CPLEX
  // int *copy_path = (int *)calloc(inst->nnodes * 2, sizeof(int));
  inst->ncols = CPXgetnumcols(env, lp);
  double *xstar = (double *)calloc(inst->ncols, sizeof(double));
  if (CPXgetx(env, lp, xstar, 0, inst->ncols - 1))
    print_error("CPXgetx() error");
  xstarToPath(inst, xstar, pow((double)inst->nnodes, 2.0), path);

#ifndef PRODUCTION
  char *tmp;
  tmp = getPath(path, inst->nnodes);
  DEBUG_COMMENT("tspcplex.c:TSPopt", "path = %s", tmp);
  free(tmp);
#endif

  free(xstar);

  // free and close cplex model
  CPXfreeprob(env, &lp);
  CPXcloseCPLEX(&env);
}

void build_sol(const double *xstar, Instance *inst, int *succ, int *comp, int *ncomp)
{

  *ncomp = 0;
  for (int i = 0; i < inst->nnodes; i++)
  {
    succ[i] = -1;
    comp[i] = -1;
  }

  for (int start = 0; start < inst->nnodes; start++)
  {
    if (comp[start] >= 0)
      continue; // node "start" was already visited, just skip it

    // a new component is found
    (*ncomp)++;
    int i = start;
    int done = 0;
    while (!done) // go and visit the current component
    {
      comp[i] = *ncomp;
      done = 1;
      for (int j = 0; j < inst->nnodes; j++)
      {
        if (i != j && xstar[xpos(i, j, inst)] > 0.5 && comp[j] == -1) // the edge [i,j] is selected in xstar and j was not visited before
        {
          succ[i] = j;
          i = j;
          done = 0;
          break;
        }
      }
    }
    succ[i] = start; // last arc to close the cycle
                     // go to the next component...
    DEBUG_COMMENT("tspcplex.c:build_model", "succ: %s", getPath(succ, inst->nnodes));
    DEBUG_COMMENT("tspcplex.c:build_model", "comp: %s", getPath(comp, inst->nnodes));
  }
}
