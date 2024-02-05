#include "../include/mycallback.h"
// #include "../concorde_build/concorde.h"
#include <concorde.h>
#include <stdlib.h>

#include "../include/refinement.h"
#include "../include/tspcplex.h"
#include "../include/utilscplex.h"

double cut_violation(double *xstar, int nnz, double rhs, char sense, int *index,
                     double *value) {
  double lhs = 0.0;
  for (int i = 0; i < nnz; i++) {
    lhs += xstar[index[i]] * value[i];
  }
  if (sense == 'L')
    return lhs - rhs;
  else if (sense == 'G')
    return rhs - lhs;
  else
    return fabs(lhs - rhs);
}

int relaxation_cuts(double cutval, int cutcount, int *cut, void *inparam) {
  DEBUG_COMMENT("mycallback.c:relaxation_cuts", "entering relaxation_cuts");
  ParamsConcorde *param = (ParamsConcorde *)inparam;

  Instance *inst = param->inst;

  int ecount = cutcount * (cutcount - 1) / 2;
  int *index = (int *)calloc(ecount, sizeof(int));
  double *value = (double *)calloc(ecount, sizeof(double));
  double rhs = cutcount - 1;
  char sense = 'L';
  int purgeable = CPX_USECUT_FILTER;
  int matbeg = 0;
  int local = 0;

  int nnz = 0;
  DEBUG_COMMENT("mycallback.c:relaxation_cuts", "cutcount = %d", cutcount);
  for (int i = 0; i < cutcount; i++) {
    for (int j = i + 1; j < cutcount; j++) {
      index[nnz] = xpos(cut[i], cut[j], inst);
      value[nnz] = 1.0;
      nnz++;
    }
  }
  DEBUG_COMMENT("mycallback.c:relaxation_cuts", "nnz = %d", nnz);
  double violation = cut_violation(param->xstar, nnz, rhs, sense, index, value);

  if (fabs(violation - (2 - cutval) / 2) > EPSILON)
    ERROR_COMMENT("tspcplex.c:relaxation_cuts", "cutval = %lf, violation = %lf",
                  cutval, violation);
  if (CPXcallbackaddusercuts(param->context, 1, nnz, &rhs, &sense, &matbeg,
                             index, value, &purgeable, &local))
    ERROR_COMMENT("tspcplex.c:relaxation_cuts", "CPXcallbackaddusercuts error");
  DEBUG_COMMENT("mycallback.c:relaxation_cuts",
                "rhs = %10.0f, nnz = %2d, numrows = %d \n", rhs, nnz,
                CPXgetnumrows(param->inst->env, param->inst->lp));

  DEBUG_COMMENT("mycallback.c:relaxation_cuts", "exiting relaxation_cuts");
  free(index);
  free(value);
  return SUCCESS;
}

// CUT CALLBACK
int my_callback_relaxation(CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
                           void *userhandle) {
  INFO_COMMENT("tspcplex.c:my_callback_relaxation",
               "entering relaxation callbacks");

  int mythread = -1;
  CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread);
  int mynode = -1;
  CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode);
  double incumbent = CPX_INFBOUND;
  CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);

  Instance *inst = (Instance *)userhandle;
  double *xstar = (double *)calloc(sizeof(double), inst->ncols);
  double objval = CPX_INFBOUND;
  if (mynode % inst->nnodes != 0) return SUCCESS;
  if (CPXcallbackgetrelaxationpoint(context, xstar, 0, inst->ncols - 1,
                                    &objval))
    ERROR_COMMENT("tspcplex.c:my_callback_relaxation",
                  "CPXcallbackgetrelaxationpoint error");
  int ncomp = 0;
  int *comp = (int *)calloc(inst->nnodes,
                            sizeof(int));  // edges pertaining to each component
  int *comp_count = (int *)calloc(inst->nnodes, sizeof(int));

  int *elist = (int *)calloc(inst->ncols * 2, sizeof(int));
  int ecount = 0;

  int loader = 0;
  for (int i = 0; i < inst->nnodes; i++) {
    for (int j = i + 1; j < inst->nnodes; j++) {
      elist[loader++] = i;
      elist[loader++] = j;
      ecount++;
    }
  }

  DEBUG_COMMENT("tspcplex.c:my_callback_relaxation",
                "calling CCcut_connect_components");
  if (CCcut_connect_components(inst->nnodes, ecount, elist, xstar, &ncomp,
                               &comp_count, &comp))
    ERROR_COMMENT("tspcplex.c:my_callback_relaxation",
                  "CCcut_connect_components error");

  DEBUG_COMMENT("tspcplex.c:my_callback_relaxation", "ncomp= %d", ncomp);

  ParamsConcorde params;
  params.inst = inst;
  params.context = context;
  params.xstar = xstar;
  // inst->params = params;

  if (ncomp == 1) {
    DEBUG_COMMENT("tspcplex.c:my_callback_relaxation", "CCcut_violated_cuts");
    // printf("§§§§§§ relaxation cust ncomp=1 §§§§§§\n");
    if (CCcut_violated_cuts(inst->nnodes, ecount, elist, xstar, 2.0 - EPSILON,
                            relaxation_cuts, &params))
      ERROR_COMMENT("tspcplex.c:my_callback_relaxation",
                    "CCcut_violated_cuts error");
  } else if (ncomp > 1) {
    // printf("§§§§§§ relaxation cust ncomp>1 §§§§§§\n");
    DEBUG_COMMENT("tspcplex.c:my_callback_relaxation", "add SEC");
    int position = 0;
    for (int i = 0; i < ncomp; i++) {
      int *succ = (int *)calloc(comp_count[i], sizeof(int));

      for (int j = 0; j < comp_count[i]; j++) {
        succ[j] = comp[position + j];
      }
      position += comp_count[i];
      double cutval = 0.0;
      relaxation_cuts(cutval, comp_count[i], succ, &params);
      free(succ);
    }
  }
  // Free allocated memory
  free(comp);
  free(comp_count);
  free(xstar);
  free(elist);
  // int status = INSTANCE_assert(inst);
  // if (status != SUCCESS)
  //   exit(status);
  return SUCCESS;
}

int my_callback_candidate(CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
                          void *userhandle) {
  INFO_COMMENT("mycallback.c:my_callback_candidate", "Entering");
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
  DEBUG_COMMENT("mycallback.c:my_callback_candidate", "ncomp = %d", ncomp);

  if (ncomp > 1) {
    // int rmatind[inst->ncols];
    // double rmatval[inst->ncols];
    int *index = (int *)calloc(inst->ncols, sizeof(int));
    double *value = (double *)calloc(inst->ncols, sizeof(double));

    char sense = 'L';  // 'L' for less than or equal to constraint
    int izero = 0;
    int nnz = 0;
    int num_nodes = 0;

    CPXwriteprob(inst->env, inst->lp, "tsppre.lp", NULL);
    // For each component
    // printf("$$$$$\n\nmycallback.c:my_callback_candidate: ncomp > 1 ||| "
    //        "%d\n\n$$$$$\n",
    //        ncomp);

    for (int k = 1; k <= ncomp; k++) {
      num_nodes = 0;
      nnz = 0;
      izero = 0;

      // printf("---------------------------------------------------\n");
      // printf("LOOKINTO comp[%d] = %d\n", k, comp[k]);
      for (int i = 0; i < inst->nnodes; i++) {
        // printf("working in comp[%d] = %d\n", i, comp[i]);

        if (comp[i] != k) continue;

        num_nodes++;

        for (int j = i + 1; j < inst->nnodes; j++) {
          if (comp[j] != k) continue;
          // printf("adding edge %d %d in comp %d \n", i, j, comp[j]);
          index[nnz] = xpos(i, j, inst);
          value[nnz] = 1.0;
          nnz++;
        }
      }
      double rhs = num_nodes - 1;

      // for (int i = 0; i < nnz; i++) {
      //   printf("index[%d] = %d, value[%d] = %lf\n", i, index[i], i,
      //   value[i]);
      // }
      // printf("nnz=%d, rhs=%lf izeor=%d\n", nnz, rhs, izero);
      if (CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &izero,
                                     index, value))
        ERROR_COMMENT("constraint.c:my_callback", "CPXaddrows(): error 1");
    }
    // printf("$$$$$\n\nmycallback.c:my_callback_candidate: ncomp > 1 ||| "
    //        "rejected candidate\n\n$$$$$\n");
    CPXwriteprob(inst->env, inst->lp, "tspafter.lp", NULL);
    free(index);
    index = NULL;
    free(value);
    value = NULL;
  } else if (ncomp == 1) {
    DEBUG_COMMENT("mycallback.c:my_callback_candidate", "DIOSCANNATO");
    RUN(xstarToPath(inst, xstar, inst->ncols, inst->path));

    RUN(two_opt_noupdate(inst, INFTY));
    INSTANCE_calculateTourLength(inst);
    RUN(INSTANCE_assert(inst));

    int indices[inst->ncols];
    double xheu[inst->ncols];

    for (int i = 0; i < inst->ncols; i++) {
      xheu[i] = 0;
      indices[i] = i;
    }

    for (int i = 0; i < inst->nnodes; i++) {
      xheu[xpos(i, succ[i], inst)] = 1;
    }

    int status =
        CPXcallbackpostheursoln(context, inst->ncols, indices, xheu,
                                inst->tour_length, CPXCALLBACKSOLUTION_NOCHECK);
    // inst->tour_length, CPXCALLBACKSOLUTION_SOLVE);

    if (status)
      ERROR_COMMENT("mycallback.c:my_callback_candidate",
                    "CPXcallbackpostheursoln error %d", status);
    //    printf("$$$$$\n\nmycallback.c:my_callback_candidate: ncomp = 1 ||| "
    //"accetted candidate tl=%lf\n\n$$$$$\n",
    // inst->tour_length);
  }

  free(xstar);
  DEBUG_COMMENT("mycallback.c:my_callback_candidate", "Exiting");
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

  return SUCCESS;
}
