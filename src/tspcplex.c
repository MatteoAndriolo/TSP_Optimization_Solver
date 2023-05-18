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
  double rhs = cutcount - 1;
  int purgeable = CPX_USECUT_FILTER;
  int local = 0;
  int izero = 0;
  int nnz = 0;
  for (int ipos = 0; ipos < cutcount; ipos++)
  {
    for (int jpos = ipos + 1; jpos < cutcount; jpos++)
    {
      index[nnz] = xpos(cut[ipos], cut[jpos], param->inst);
      value[nnz] = 1.0;
      nnz++;
    }
    // TODO: go single thread clone the model env(env2, lp2) put the poiters in the instance data structure when I am in a callback add subtour elimination constrain addrowsfunction and print the model
  }
  if (CPXcallbackaddusercuts(param->context, 1, nnz, &rhs, &sense, &izero, index, value, &purgeable, &local))
    ERROR_COMMENT("tspcplex.c:doit_fn_concorde", "CPXcallbackaddusercuts() error");
  printf("rhs = %10.0f, nnz = %2d, cutcount = %d \n", rhs, nnz, cutcount);
  free(value);
  free(index);
  return 0;
}

int my_callback_relaxation(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle)
{
  printf("my_callback_relaxation\n");
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
      // TODO:check with xpos
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
  printf("ncomp = %d\n", ncomp);
  if (ncomp == 1)
  {
    DEBUG_COMMENT("tspcplex.c:my_callback_relaxation", "inside the if condition on the relaxation point, we are in cause the number of the connected component, ncomp = %d", ncomp);
    Input params;
    params.inst = inst;
    params.context = context;

    if (CCcut_violated_cuts(inst->nnodes, ecount, elist, xstar, 2.0 - EPSILON, doit_fn_concorde, &params))
      print_error("CCcut_violated_cuts error");
  }
  // TODO: check fuction for != ncomp
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

int hard_fixing(CPXENVptr env, CPXLPptr lp, Instance *inst, int *path)
{
  DEBUG_COMMENT("tspcplex.c:hard_fixing", "entering hard fixing");
  //----------------------------------------- set the time limit to run cplex ---------------------------------------------------------
  // TODO: create a function set the param of cplex
  CPXsetdblparam(env, CPXPARAM_TimeLimit, 15);
  //----------------------------------------- create the xheuristic soltion from a warm start ----------------------------------------
  double *xheu = (double *)calloc(inst->ncols, sizeof(double));
  create_xheu(inst, xheu, path);
  generate_mip_start(inst, env, lp, xheu);

  double best_solution = INFTY;
  int no_improv = 0;
  while (no_improv < 1)
  {
    //----------------------------------------- fixing some edges   ---------------------------------------------------------------
    fix_edges(env, lp, inst, xheu);
#ifndef PRODUCTION
    CPXwriteprob(env, lp, "mipopt.lp", NULL);
#endif
    //--------------------------------- calling the branch and cut solution -------------------------------------------------------
    inst->solver = 2;
    int status = solve_problem(env, lp, inst, path); // branch_and_cut(inst, env, lp, CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_RELAXATION);
    if (status)
      print_error("Execution FAILED");
    // ---------------------------------- check if the solution is better than the previous one ---------------------------------
    double actual_solution;
    int error = CPXgetobjval(env, lp, &actual_solution);
    if (error)
      print_error("CPXgetobjval() error\n");

    if (actual_solution < best_solution)
    { // UPDATE path
      if (CPXgetx(env, lp, xheu, 0, inst->ncols - 1))
        print_error("CPXgetx() error");
      best_solution = actual_solution;
      xstarToPath(inst, xheu, inst->ncols, path);
    }
    //----------------------------------------- unfixing  edges   ---------------------------------------------------------------
    unfix_edges(env, lp, inst, xheu);

    if (actual_solution >= best_solution)
      break;
  }

  inst->solver = 2;
  solve_problem(env, lp, inst, path);
  if (CPXgetx(env, lp, xheu, 0, inst->ncols - 1))
    print_error("CPXgetx() error");
  xstarToPath(inst, xheu, inst->ncols, path);

  free(xheu);
  return 0;
}

int local_branching(CPXENVptr env, CPXLPptr lp, Instance *inst, int *path)
{
  //----------------------------------------- set the time limit to run cplex ---------------------------------------------------------
  // TODO: create a function set the param of cplex
  CPXsetdblparam(env, CPXPARAM_TimeLimit, 15);
  //----------------------------------------- create the xheuristic soltion from a warm start ----------------------------------------
  double *xheu = (double *)calloc(inst->ncols, sizeof(double));
  create_xheu(inst, xheu, path);
  generate_mip_start(inst, env, lp, xheu);

  double best_solution = INFTY;
  int no_improv = 0;
  while (no_improv < 1)
  {
    //------------------------------------------ take current solution and eliminate some solutions -------------------------------

    eliminate_radius_edges(env, lp, inst, xheu, 30);
#ifndef PRODUCTION
    CPXwriteprob(env, lp, "mipopt.lp", NULL);
#endif
    //--------------------------------- calling the branch and cut solution -------------------------------------------------------
    int status = branch_and_cut(inst, env, lp, CPX_CALLBACKCONTEXT_CANDIDATE);
    if (status)
      print_error("Execution FAILED");

    if (CPXgetx(env, lp, xheu, 0, inst->ncols - 1))
      print_error("CPXgetx() error");
    // ---------------------------------- check if the solution is better than the previous one ---------------------------------
    double actual_solution;
    int error = CPXgetobjval(env, lp, &actual_solution);
    if (error)
      print_error("CPXgetobjval() error\n");
    //---------------------------------- Unfix the edges and calling thesolution --------------------------------------------------
    repristinate_radius_edges(env, lp, inst, xheu);
    // CPXwriteprob(env, lp, "mipopt.lp", NULL);

    if (actual_solution >= best_solution)
      no_improv++;
    else
    {
      best_solution = actual_solution;
      no_improv = 0;
      xstarToPath(inst, xheu, inst->ncols, path);
    }
  }
  //--------------------------------- calling the branch and cut solution -------------------------------------------------------
  repristinate_radius_edges(env, lp, inst, xheu);
  int status = branch_and_cut(inst, env, lp, CPX_CALLBACKCONTEXT_CANDIDATE);
  if (status)
    print_error("Execution FAILED");
  if (CPXgetx(env, lp, xheu, 0, inst->ncols - 1))
    print_error("CPXgetx() error");
  xstarToPath(inst, xheu, inst->ncols, path);
  free(xheu);
  return 0;
}

int solve_problem(CPXENVptr env, CPXLPptr lp, Instance *inst, int *path)
{
  DEBUG_COMMENT("tspcplex.c:solve_problem", "entering solve problem");
  int status = -1;
  printf("/////////////////////////////////////////////////////////////////////////////////");
  printf("solver = %d\n", inst->solver);
  printf("/////////////////////////////////////////////////////////////////////////////////");
  // TODO: fix the solver function cplexheu and cplex
  // CPXwriteprob(env, lp, "mipopt.lp", NULL);
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
    status = hard_fixing(env, lp, inst, path);
  }
  else if (inst->solver == 5)
  {
    status = local_branching(env, lp, inst, path);
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
  int status = solve_problem(env, lp, inst, path);
  if (status)
    print_error("Execution FAILED");
  else
    printf("status = %d-------------------------------------------s\n", status);

#ifndef PRODUCTION
  //-----------------------------------------getting the solution-----------------------------------------------------------------
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
  //------------------------ free and close cplex model --------------------------------------------------------------
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
