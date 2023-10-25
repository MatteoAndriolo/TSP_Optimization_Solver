#include "../include/mycallback.h"
// #include "../concorde_build/concorde.h"
#include <concorde.h>
#include "../include/refinement.h"
#include "../include/tspcplex.h"
#include "../include/utilscplex.h"
#include <stdlib.h>

int relaxation_cuts(double cutval, int cutcount, int *cut, void *inparam)
{
  DEBUG_COMMENT("mycallback.c:relaxation_cuts", "entering relaxation_cuts");
  ParamsConcorde *param = (ParamsConcorde *)inparam;
  Instance *inst = param->inst;
  // CPXCALLBACKCONTEXTptr context = param->context;
  //  double objval = CPX_INFBOUND;
  int index[inst->ecount];
  double value[inst->ecount];
  char sense = 'L';
  double rhs = cutcount - 1;
  int purgeable = CPX_USECUT_FILTER;
  int matbeg = 0;
  int local = 0;

  int nnz = 0;
  DEBUG_COMMENT("mycallback.c:relaxation_cuts", "cutcount = %d", cutcount);
  for (int i = 0; i < cutcount; i++)
  {
    for (int j = i + 1; j < cutcount; j++)
    {
      index[nnz] = xpos(cut[i], cut[j], inst);
      value[nnz] = 1.0;
      nnz++;
    }
  }
  DEBUG_COMMENT("mycallback.c:relaxation_cuts", "nnz = %d", nnz);

  if (CPXcallbackaddusercuts(param->context, 1, nnz, &rhs, &sense, &matbeg,
                             index, value, &purgeable, &local))
    ERROR_COMMENT("tspcplex.c:relaxation_cuts", "CPXcallbackaddusercuts error");
  DEBUG_COMMENT("mycallback.c:relaxation_cuts",
                "rhs = %10.0f, nnz = %2d, numrows = %d \n", rhs, nnz,
                CPXgetnumrows(param->inst->env, param->inst->lp));

  DEBUG_COMMENT("mycallback.c:relaxation_cuts", "exiting relaxation_cuts");
  return SUCCESS;
}

// int doit_fn_concorde(double cutval, int cutcount, int *cut, void *inparam) {
//   // Input *param = (Input *)inparam;
//   ParamsConcorde *param = (ParamsConcorde *)inparam;
//   Instance *inst = param->inst;
//   CPXCALLBACKCONTEXTptr context = param->context;
//   double *xstart = param->xstar;
//
//   int ecount = inst->nnodes * (inst->nnodes - 1) / 2;
//   double *value = (double *)calloc(ecount, sizeof(double));
//   int *index = (int *)calloc(ecount, sizeof(int));
//   char sense = 'L';
//   double rhs = cutcount - 1;
//   int purgeable = CPX_USECUT_FILTER;
//   int local = 0;
//   int izero = 0;
//   int nnz = 0;
//   for (int ipos = 0; ipos < cutcount; ipos++) {
//     for (int jpos = ipos + 1; jpos < cutcount; jpos++) {
//       index[nnz] = xpos(cut[ipos], cut[jpos], inst);
//       value[nnz] = 1.0;
//       nnz++;
//     }
//     // TODO: go single thread clone the model env(env2, lp2) put the poiters
//     // the instance data structure when I am in a callback add subtour
//     // elimination constrain addrowsfunction and print the model } if
//     (CPXcallbackaddusercuts(context, 1, nnz, &rhs, &sense, &izero, index,
//                              value, &purgeable, &local))
//     ERROR_COMMENT("tspcplex.c:doit_fn_concorde",
//                   "CPXcallbackaddusercuts() error");
//   printf("rhs = %10.0f, nnz = %2d, cutcount = %d \n", rhs, nnz, cutcount);
//   free(value);
//   free(index);
//   return 0;
// }

// CUT CALLBACK
int my_callback_relaxation(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle)
{
  INFO_COMMENT("tspcplex.c:my_callback_relaxation", "entering relaxation callbacks");
  Instance *inst = (Instance *)userhandle;
  double *xstar = (double *)malloc(sizeof(double) * inst->ncols);
  double objval = CPX_INFBOUND;
  int ncomp = 0;
  int **comp = (int **)malloc(inst->nnodes * sizeof(int *)); // edges pertaining to each component
  int **comp_count = (int **)malloc(inst->nnodes * sizeof(int *));
  for (int i = 0; i < inst->nnodes; i++)
  {
    comp_count[i] = (int *)calloc(1, sizeof(int));      // Allocate memory for a single integer
    comp[i] = (int *)calloc(inst->nnodes, sizeof(int)); // Allocate memory for a single integer
  }

  int *elist = (int *)malloc(inst->ncols * 2 * sizeof(int));
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

  if (CPXcallbackgetrelaxationpoint(context, xstar, 0, inst->ncols - 1, &objval))
    ERROR_COMMENT("tspcplex.c:my_callback_relaxation", "CPXcallbackgetrelaxationpoint error");

  int mythread = -1;
  CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread);
  int mynode = -1;
  CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode);
  double incumbent = CPX_INFBOUND;
  CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);

  DEBUG_COMMENT("tspcplex.c:my_callback_relaxation", "calling CCcut_connect_components");
  if (CCcut_connect_components(inst->nnodes, ecount, elist, xstar, &ncomp, comp_count, comp))
    ERROR_COMMENT("tspcplex.c:my_callback_relaxation", "CCcut_connect_components error");
  int position = 0;
  // print com_count and comp
  for (int i = 0; i < ncomp; i++)
  {
    DEBUG_COMMENT("tspcplex.c:my_callback_relaxation", "comp_count[0][%d] = %d", i, comp_count[0][i]);
    for (int j = 0; j < comp_count[0][i]; j++)
    {
      DEBUG_COMMENT("tspcplex.c:my_callback_relaxation", "comp[%d][%d] = %d", 0, j, comp[0][position + j]);
    }
    position += comp_count[0][i];
  }

  DEBUG_COMMENT("tspcplex.c:my_callback_relaxation", "ncomp= %d", ncomp);

  ParamsConcorde *params = (ParamsConcorde *)malloc(sizeof(ParamsConcorde));
  params->inst = inst;
  params->context = context;
  params->xstar = xstar;
  inst->params = params;

  if (ncomp == 1)
  {
    DEBUG_COMMENT("tspcplex.c:my_callback_relaxation", "CCcut_violated_cuts");
    if (CCcut_violated_cuts(inst->nnodes, ecount, elist, xstar, 2.0 - EPSILON, relaxation_cuts, &params))
      ERROR_COMMENT("tspcplex.c:my_callback_relaxation", "CCcut_violated_cuts error");
  }
  else if (ncomp > 1)
  {
    DEBUG_COMMENT("tspcplex.c:my_callback_relaxation", "add SEC");

    for (int i = 0; i < ncomp; i++)
    {
      int *succ = (int *)calloc(*comp_count[0], sizeof(int));

      for (int j = 0; j < comp_count[0][i]; j++)
      {
        succ[j] = comp[i][j];
      }
      double cutval = 0.0;
      relaxation_cuts(cutval, comp_count[0][i], succ, params);
      free(succ);
    }
  }
  // Free allocated memory
  for (int i = 0; i < inst->nnodes; i++)
  {
    free(comp[i]);
    free(comp_count[i]);
  }
  free(comp);
  free(comp_count);
  free(xstar);
  free(elist);
  free(params);

  return SUCCESS;
}

int my_callback_candidate(CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
                          void *userhandle)
{
  DEBUG_COMMENT("mycallback.c:my_callback_candidate", "Entering");
  Instance *inst = (Instance *)userhandle;
  double *xstar = (double *)malloc(sizeof(double) * inst->ncols);
  double objval = CPX_INFBOUND;
  if (CPXcallbackgetcandidatepoint(context, xstar, 0, inst->ncols - 1, &objval))
    ERROR_COMMENT("tspcplex.c:my_callback_candidate",
                  "CPXcallbackgetcandidatepoint error");

  int mythread = -1;
  CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread);
  int mynode = -1;
  CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode);
  double incumbent = CPX_INFBOUND;
  CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);

  int succ[inst->nnodes];
  int comp[inst->nnodes];
  int ncomp = 0;

  build_sol(xstar, inst, succ, comp, &ncomp);
  // xstarToPath(inst, xstar, inst->ncols, inst->path); // done in ncomp==1
  DEBUG_COMMENT("mycallback.c:my_callback_candidate", "ncomp = %d", ncomp);

  if (ncomp > 1)
  {
    int rmatind[inst->ncols];
    double rmatval[inst->ncols];
    char sense = 'L'; // 'L' for less than or equal to constraint
    int izero = 0;
    int nnz = 0;

    int comp_nodes[inst->nnodes];
    int size_comp;

    // For each component
    for (int i = 0; i < inst->nnodes; i++)
    {
      if (comp[i] == -1)
        continue;

      int current_component = comp[i];
      size_comp = 0;

      // Build list of nodes in component
      for (int j = i; j < inst->nnodes; j++)
      {
        if (comp[j] == current_component)
        {
          comp_nodes[size_comp++] = j;
          comp[j] = -1;
        }
      }

      if (size_comp <= 2)
        continue;

      // build index and value
      for (int j = 0; j < size_comp; j++)
      {
        for (int k = j + 1; k < size_comp; k++)
        {
          rmatind[nnz] = xpos(comp_nodes[j], comp_nodes[k], inst);
          rmatval[nnz] = 1.0;
          nnz++;
        }
      }
      double rhs = nnz - 1;

      if (CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &izero,
                                     rmatind, rmatval))
        ERROR_COMMENT("constraint.c:my_callback", "CPXaddrows(): error 1");
    }
  }
  else if (ncomp == 1)
  {
    RUN(xstarToPath(inst, xstar, inst->ncols, inst->path));
    inst->tour_length = INSTANCE_calculateTourLength(inst);
    two_opt(inst, 5 * inst->nnodes);

    int indices[inst->ncols];
    double xheu[inst->ncols];
    for (int i = 0; i < inst->ncols; i++)
    {
      xheu[i] = 0;
      indices[i] = i;
    }

    for (int i = 0; i < inst->nnodes; i++)
    {
      xheu[xpos(i, succ[i], inst)] = 1;
    }

    int status =
        CPXcallbackpostheursoln(context, inst->ncols, indices, xheu,
                                inst->tour_length, CPXCALLBACKSOLUTION_NOCHECK);
    if (status)
      ERROR_COMMENT("mycallback.c:my_callback_candidate",
                    "CPXcallbackpostheursoln error %d", status);
  }

  // free(succ);
  // free(comp);
  free(xstar);
  DEBUG_COMMENT("mycallback.c:my_callback_candidate", "Exiting");
  return 0;
}

//-----correspond to sec_callback---------
int CPXPUBLIC my_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
                          void *userhandle)
{
  INFO_COMMENT("tspcplex.c:my_callback",
               "entering callbacks function here we are going to decide ");
  Instance *inst = (Instance *)userhandle;

  if (contextid == CPX_CALLBACKCONTEXT_RELAXATION)
    return my_callback_relaxation(context, contextid, inst);
  if (contextid == CPX_CALLBACKCONTEXT_CANDIDATE)
    return my_callback_candidate(context, contextid, inst);

  return SUCCESS;
}
