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

int doit_fn_concorde(double cutval, int cutcount, int *cut, void *inparam)
{
  Input *param = (Input *)inparam;
  int ecount = param->inst->nnodes * (param->inst->nnodes - 1) / 2;
  double *value = (double *)calloc(ecount, sizeof(double));
  int *index = (int *)calloc(ecount, sizeof(int));
  char sense = 'L';
  double rhs = param->inst->nnodes - 1;
  int purgeable = CPX_USECUT_FILTER;
  int local = 0;
  int izero = 0;
  int nnz = 0;
  for (int i = 0; i < param->inst->nnodes; ++i)
  {
    for (int j = i + 1; j < param->inst->nnodes; ++j)
    {
      if (cut[i] != cut[j])
      {
        index[nnz] = xpos(cut[i], cut[j], param->inst);
        value[nnz] = 1.0;
        nnz++;
      }
    }
  }
  if (CPXcallbackaddusercuts(param->context, 1, nnz, &rhs, &sense, &izero, index, value, &purgeable, &local))
    ERROR_COMMENT("tspcplex.c:doit_fn_concorde", "CPXcallbackaddusercuts() error");
  free(value);
  free(index);
  return 0;
}

int my_callback_relaxation(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle)
{
  INFO_COMMENT("tspcplex.c:my_callback_relaxation", "entering relaxation callbacks");
  Instance *inst = (Instance *)userhandle;
  double *xstar = (double *)malloc(sizeof(double) * inst->ncols);
  double objval = CPX_INFBOUND;
  int ncomp = 0;
  int *comps = (int *)calloc(inst->nnodes, sizeof(int));      // edges pertaining to each component
  int *compscount = (int *)calloc(inst->nnodes, sizeof(int)); // number of nodes in each component
  int *elist = (int *)calloc(inst->ncols * 2, sizeof(int));   // list of edges
  int loader = 0;
  int ecount = 0;
  for (int i = 0; i < inst->nnodes; i++)
  {
    for (int j = i + 1; j < inst->nnodes; j++)
    {
      elist[loader++] = i;
      elist[loader++] = j;
      ecount++;
    }
  }
  //----------------------check if the solution work and che some information--------------------------------
  if (CPXcallbackgetrelaxationpoint(context, xstar, 0, inst->ncols - 1, &objval))
    print_error("CPXcallbackgetcandidatepoint error");
  int mythread = -1;
  CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread);
  int mynode = -1;
  CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode);
  double incumbent = CPX_INFBOUND;
  CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);
  //------------------------start using concorde------------------------------------------------------------
  INFO_COMMENT("tspcplex.c:my_callback_relaxation", "calling CCcut_connect_components");
  if (CCcut_connect_components(inst->nnodes, ecount, elist, xstar, &ncomp, &compscount, &comps))
    print_error("CCcut_connect_components error");

  if (ncomp == 1)
  {
    DEBUG_COMMENT("tspcplex.c:my_callback_relaxation", "inside the if condition on the relaxation point, we are in cause the number of the connected component, ncomp = %d", ncomp);
    Input params;
    params.inst = inst;
    params.context = context;

    if (CCcut_violated_cuts(inst->nnodes, ecount, elist, xstar, 2.0 - EPSILON, doit_fn_concorde, &params))
      print_error("CCcut_violated_cuts error");
  }
  free(xstar);
  free(comps);
  free(compscount);
  free(elist);
  return 0;
}

int my_callback_candidate(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle)
{
  Instance *inst = (Instance *)userhandle;
  int *succ = (int *)malloc(sizeof(int) * inst->nnodes);
  int *comp = (int *)malloc(sizeof(int) * inst->nnodes);
  double *xstar = (double *)malloc(sizeof(double) * inst->ncols);
  double objval = CPX_INFBOUND;
  int ncomp;
  //----------------------check if the solution work and che some information--------------------------------
  if (CPXcallbackgetcandidatepoint(context, xstar, 0, inst->ncols - 1, &objval))
    print_error("CPXcallbackgetcandidatepoint error");
  int mythread = -1;
  CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread);
  int mynode = -1;
  CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode);
  double incumbent = CPX_INFBOUND;
  CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);
  //-----------------------buil the solution-----------------------------------------------------------------
  build_sol(xstar, inst, succ, comp, &ncomp);
  //--------------------------------add the sec's cut--------------------------------------------------------
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
      int izero = 0;
      double rsh = nncc - 1;
      if (CPXcallbackrejectcandidate(context, 1, nnz, &rsh, &sense, &izero, index, value))
        ERROR_COMMENT("constraint.c:my_callback", "CPXaddrows(): error 1");
      free(index);
      free(value);
    }
  }
  free(succ);
  free(comp);
  free(xstar);
  return 0;
}

//-----correspond to sec_callback---------
int CPXPUBLIC my_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle)
{
  INFO_COMMENT("tspcplex.c:my_callback", "entering callbacks function here we are going to decide ");
  Instance *inst = (Instance *)userhandle;

  if (contextid == CPX_CALLBACKCONTEXT_RELAXATION)
    return my_callback_relaxation(context, contextid, inst);
  if (contextid == CPX_CALLBACKCONTEXT_CANDIDATE)
    return my_callback_candidate(context, contextid, inst);

  return 0;
}

int branch_and_cut(Instance *inst, CPXENVptr env, CPXLPptr lp, CPXLONG contextid)
{
  if (CPXcallbacksetfunc(env, lp, contextid, my_callback, inst))
    print_error("CPXcallbacksetfunc() error");

  CPXmipopt(env, lp);

  double *xstar = (double *)calloc(inst->ncols, sizeof(double));
  CPXgetx(env, lp, xstar, 0, inst->ncols - 1);

  int *succ = (int *)calloc(inst->nnodes, sizeof(int));
  int *comp = (int *)calloc(inst->nnodes, sizeof(int));
  int ncomp;

  build_sol(xstar, inst, succ, comp, &ncomp);

  double z;
  int error = CPXgetobjval(env, lp, &z);
  if (error)
    print_error("CPXgetobjval() error\n");

  free(comp);
  free(succ);
  free(xstar);

  return 0;
}

int solve_problem(CPXENVptr env, CPXLPptr lp, Instance *inst)
{
  int status;
  INFO_COMMENT("tspcplex.c:solve_problem", "Solving problem called here we are going to decide which solver to use");
  inst->solver = 2;
  CPXwriteprob(env, lp, "mipopt.lp", NULL);
  if (inst->solver == 1)
  {
    status = add_subtour_constraints(inst, env, lp);
  }
  else if (inst->solver == 2)
  {
    status = branch_and_cut(inst, env, lp, CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_RELAXATION);
  }
  else if (inst->solver == 3)
  {
    status = branch_and_cut(inst, env, lp, CPX_CALLBACKCONTEXT_CANDIDATE);
  }
  else if (inst->solver == 4)
  {
    // implmenetation hard fixing here
  }
  else
  {
    print_error("Invalid solver selected");
  }

  if (status)
    print_error("Execution FAILED");
  else if (status == 2)
    print_error("Time out during execution");
  return status;
}

void TSPopt(Instance *inst, int *path)
{
  INFO_COMMENT("tspcplex.c:TSPopt", "Solving TSP with CPLEX");
  int error;
  CPXENVptr env = CPXopenCPLEX(&error);
  if (error)
    print_error("CPXopenCPLEX() error");
  CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
  if (error)
    print_error("CPXcreateprob() error");

  //------------------------------------------ Building the model----------------------------------------------------------------
  build_model(inst, env, lp);
  INFO_COMMENT("tspcplex.c:TSPopt", "Model built FINISHED");
  //----------------------------------------- Cplex's parameter setting ---------------------------------------------------------
  CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
  CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 123456);
  CPXsetdblparam(env, CPX_PARAM_TILIM, inst->timelimit);
  //-----------------------------------------computing the solution---------------------------------------------------------------
  int status = solve_problem(env, lp, inst);
  if (status)
    print_error("Execution FAILED");
    //-----------------------------------------getting the solution-----------------------------------------------------------------
#ifndef PRODUCTION
  inst->ncols = CPXgetnumcols(env, lp);
  double *xstar = (double *)calloc(inst->ncols, sizeof(double));
  if (CPXgetx(env, lp, xstar, 0, inst->ncols - 1))
    print_error("CPXgetx() error");
  xstarToPath(inst, xstar, pow((double)inst->nnodes, 2.0), path);

  char *tmp;
  tmp = getPath(path, inst->nnodes);
  DEBUG_COMMENT("tspcplex.c:TSPopt", "path = %s", tmp);
  free(tmp);
#endif
  // ------------------------ free and close cplex model --------------------------------------------------------------
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
