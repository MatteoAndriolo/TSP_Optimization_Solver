#include "../include/mycallback.h"
#include "../concorde_build/concorde.h"
#include "../include/utilscplex.h"

int doit_fn_concorde(double cutval, int cutcount, int *cut, void *inparam) {
  //  // Input *param = (Input *)inparam;
  //  Params_CC *param = (Params_CC *)inparam;
  //  Instance *inst = param->inst;
  //  CPXCALLBACKCONTEXTptr context = param->context;
  //
  //  int ecount = inst->nnodes * (inst->nnodes - 1) / 2;
  //  double *value = (double *)calloc(ecount, sizeof(double));
  //  int *index = (int *)calloc(ecount, sizeof(int));
  //  char sense = 'L';
  //  double rhs = cutcount - 1;
  //  int purgeable = CPX_USECUT_FILTER;
  //  int local = 0;
  //  int izero = 0;
  //  int nnz = 0;
  //  for (int ipos = 0; ipos < cutcount; ipos++) {
  //    for (int jpos = ipos + 1; jpos < cutcount; jpos++) {
  //      index[nnz] = xpos(cut[ipos], cut[jpos], inst);
  //      value[nnz] = 1.0;
  //      nnz++;
  //    }
  //    // TODO: go single thread clone the model env(env2, lp2) put the poiters
  //    in
  //    // the instance data structure when I am in a callback add subtour
  //    // elimination constrain addrowsfunction and print the model
  //  }
  //  if (CPXcallbackaddusercuts(context, 1, nnz, &rhs, &sense, &izero, index,
  //                             value, &purgeable, &local))
  //    ERROR_COMMENT("tspcplex.c:doit_fn_concorde",
  //                  "CPXcallbackaddusercuts() error");
  //  printf("rhs = %10.0f, nnz = %2d, cutcount = %d \n", rhs, nnz, cutcount);
  //  free(value);
  //  free(index);
  return 0;
}

// CUT CALLBACK
int my_callback_relaxation(CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
                           void *userhandle) {
  //   printf("my_callback_relaxation\n");
  //   INFO_COMMENT("tspcplex.c:my_callback_relaxation",
  //                "entering relaxation callbacks");
  //   Instance *inst = (Instance *)userhandle;
  //   // double *xstar = (double *)malloc(sizeof(double) * inst->ncols);
  //   double xstar[inst->ncols];
  //   double objval = CPX_INFBOUND;
  //   int ncomp = 0;
  //   // edges pertaining to each component
  //   int *comps = (int *)calloc(inst->nnodes, sizeof(int));
  //   // int comps[inst->nnodes];
  //   // number of nodes in each component
  //   int *compscount = (int *)calloc(inst->nnodes, sizeof(int));
  //
  //   //----------------------check if the solution work and che some
  //   if (CPXcallbackgetrelaxationpoint(context, xstar, 0, inst->ncols - 1,
  //                                     &objval))
  //     ERROR_COMMENT("tspcplex.c:my_callback_relaxation",
  //                   "CPXcallbackgetrelaxationpoint error");
  //   int mythread = -1;
  //   CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread);
  //   int mynode = -1;
  //   CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode);
  //   double incumbent = CPX_INFBOUND;
  //   CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);
  //   //------------------------start using
  //   // concorde------------------------------------------------------------
  //   INFO_COMMENT("tspcplex.c:my_callback_relaxation",
  //                "calling CCcut_connect_components");
  //   // int *elist = (int *)calloc(inst->ncols * 2, sizeof(int)); // list of
  //   edges int elist[inst->ncols * 2]; // list of edges int loader = 0; int
  //   ecount = 0;
  //   for (int i = 0; i < inst->nnodes; i++)
  //     for (int j = i + 1; j < inst->nnodes; j++) {
  //       // TODO:check with xpos
  //       elist[loader++] = i;
  //       elist[loader++] = j;
  //       ecount++;
  //     }
  //   if (CCcut_connect_components(inst->nnodes, ecount, elist, xstar, &ncomp,
  //                                &compscount, &comps))
  //     ERROR_COMMENT("tspcplex.c:my_callback_relaxation",
  //                   "CCcut_connect_components error");
  //   printf("ncomp = %d\n", ncomp);
  //   if (ncomp != 1) // TODO separate components here
  //   {
  //     DEBUG_COMMENT("tspcplex.c:my_callback_relaxation",
  //                   "inside the if condition on the relaxation point, we are
  //                   in " "cause the number of the connected component, ncomp
  //                   = %d", ncomp);
  //     if (CCcut_violated_cuts(inst->nnodes, ecount, elist, xstar, 2.0 -
  //     EPSILON,
  //                             doit_fn_concorde, context))
  //       ERROR_COMMENT("tspcplex.c:my_callback_relaxation",
  //                     "CCcut_violated_cuts error");
  //   }
  //   // TODO: check fuction for != ncomp
  //   // free(xstar);
  //   free(comps);
  //   free(compscount);
  //   // free(elist);
  return 0;
}

int my_callback_candidate(CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
                          void *userhandle) {
  //  Instance *inst = (Instance *)userhandle;
  //  int *succ = (int *)malloc(sizeof(int) * inst->nnodes);
  //  int *comp = (int *)malloc(sizeof(int) * inst->nnodes);
  //  double *xstar = (double *)malloc(sizeof(double) * inst->ncols);
  //  double objval = CPX_INFBOUND;
  //  int ncomp;
  //  //----------------------check if the solution work and che some
  //  // information--------------------------------
  //  if (CPXcallbackgetcandidatepoint(context, xstar, 0, inst->ncols - 1,
  //  &objval))
  //    ERROR_COMMENT("tspcplex.c:my_callback_candidate",
  //                  "CPXcallbackgetcandidatepoint error");
  //  int mythread = -1;
  //  CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread);
  //  int mynode = -1;
  //  CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode);
  //  double incumbent = CPX_INFBOUND;
  //  CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);
  //  //-----------------------buil the
  //  //
  //  solution-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
  //  -- -- -- -- -- -- -- -- -- -- -build_sol(
  //      xstar, inst, succ, comp, &ncomp);
  //  //--------------------------------add the sec's
  //  // cut--------------------------------------------------------
  //  if (ncomp > 1) {
  //    for (int cc = 1; cc <= ncomp; cc++) {
  //      int nncc = 0;
  //      for (int i = 0; i < inst->nnodes; i++) {
  //        if (comp[i] == cc)
  //          nncc++;
  //      }
  //
  //      int *index =
  //          (int *)calloc((inst->nnodes * (inst->nnodes - 1)) / 2,
  //          sizeof(int));
  //      double *value = (double *)calloc((inst->nnodes * (inst->nnodes - 1)) /
  //      2,
  //                                       sizeof(double));
  //      char sense = 'L'; // 'L' for less than or equal to constraint
  //      int nnz = 0;
  //
  //      int j = 0;
  //      int k;
  //      while (j < inst->nnodes - 1) {
  //        while (comp[j] != cc && j < inst->nnodes) {
  //          j++;
  //        }
  //        k = j + 1;
  //        while (k < inst->nnodes) {
  //          if (comp[k] == cc) {
  //            index[nnz] = xpos(j, k, inst);
  //            value[nnz] = 1.0;
  //            nnz++;
  //          }
  //          k++;
  //        }
  //        j++;
  //      }
  //      int izero = 0;
  //      double rsh = nncc - 1;
  //      if (CPXcallbackrejectcandidate(context, 1, nnz, &rsh, &sense, &izero,
  //                                     index, value))
  //        ERROR_COMMENT("constraint.c:my_callback", "CPXaddrows(): error 1");
  //      free(index);
  //      free(value);
  //    }
  //  }
  //  free(succ);
  //  free(comp);
  //  free(xstar);
  return 0;
}

//-----correspond to sec_callback---------
int CPXPUBLIC my_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
                          void *userhandle) {
  INFO_COMMENT("tspcplex.c:my_callback",
               "entering callbacks function here we are going to decide ");
  Instance *inst = (Instance *)userhandle;

  if (contextid == CPX_CALLBACKCONTEXT_RELAXATION)
    return my_callback_relaxation(context, contextid, inst);
  if (contextid == CPX_CALLBACKCONTEXT_CANDIDATE)
    return my_callback_candidate(context, contextid, inst);

  return 0;
}
