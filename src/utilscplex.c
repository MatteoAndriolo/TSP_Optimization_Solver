#include "../include/utilscplex.h"

// TODO thraw away
int cmpfunc(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

bool checkxstart(double *xstar, int dim_xstar) {
  for (int i = 0; i < dim_xstar; i++) {
    DEBUG_COMMENT("utilscplex.c:checkxstart", "xstar[%d] = %lf", i, xstar[i]);
    if (!(xstar[i] > 0 - EPSILON || xstar[i] < 1 + EPSILON)) {
      return false;
    }
  }
  return true;
}
////////////////////

int xpos(int i, int j, Instance *inst) {
  if (i == j) ERROR_COMMENT("utilscplex.c:xpos", "i == j");
  if (i > j) return xpos(j, i, inst);
  int pos = i * inst->nnodes + j - ((i + 1) * (i + 2)) / 2;
  return pos;
}

void saveBestSolution(const CPXENVptr env, const CPXLPptr lp, Instance *inst) {
  double objval;
  double *xcurr = (double *)malloc(inst->ncols * sizeof(double));
  CPXgetx(env, lp, xcurr, 0, inst->ncols - 1);
  CPXgetobjval(env, lp, &objval);

  if (objval < inst->best_tourlength) {
    DEBUG_COMMENT("utilscplex.c:getNewSolution", "New best solution found: %lf",
                  objval);

    inst->best_tourlength = objval;
    memcpy(inst->edgeList, xcurr, inst->ncols * sizeof(double));
  }
}

void xstarToPath(Instance *inst, const double *xstar, int dim_xstar,
                 int *path) {
  DEBUG_COMMENT("utilscplex.c:xstarToPath", "xstarToPath");

  double *temp_path = malloc(inst->nnodes * 2 * sizeof(double));
  memcpy(temp_path, xstar, 2 * inst->nnodes * sizeof(double));
  qsort(temp_path, inst->nnodes, sizeof(int), cmpfunc);
  DEBUG_COMMENT("utilscplex.c:xstarToPath", "Path: ");
  int count = 0;
  for (int i = 0; i < dim_xstar; i++) {
    if (temp_path[i] > 0.5) {
      count++;
      DEBUG_COMMENT("utilscplex.c:xstarToPath", "%d/%d | %d -> %lf ", i,
                    dim_xstar, count, temp_path[i]);
    }
  }
  free(temp_path);
  temp_path = NULL;

  int *succ = (int *)malloc(sizeof(int) * inst->nnodes);
  int *comp = (int *)malloc(sizeof(int) * inst->nnodes);
  int ncomp = 0;

  build_sol(xstar, inst, succ, comp, &ncomp);

  DEBUG_COMMENT("utilscplex.c:xstarToPath", "ncomp = %d", ncomp);
  for (int i = 0; i < inst->nnodes; i++) {
    DEBUG_COMMENT("utilscplex.c:xstarToPath", "%d\t%d\t%d", i, succ[i],
                  comp[i]);
  }

  int *new_path = (int *)malloc(sizeof(int) * inst->nnodes);
  int size = 0;
  int succ_node;

  for (int i = 0; i < inst->nnodes; i++) {
    new_path[size] = i;
    size++;
    succ_node = succ[i];
    comp[i] = -1;
    succ[i] = -1;
    while (succ_node != -1) {
      new_path[size] = succ_node;
      comp[new_path[size]] = -1;
      succ_node = succ[succ_node];
      succ[new_path[size]] = -1;
      size++;
    }
  }

  DEBUG_COMMENT("utilscplex.c:xstarToPath", "Path:");
  for (int i = 0; i < inst->nnodes; i++) {
    DEBUG_COMMENT("utilscplex.c:xstarToPath", "%d -> %d", i, new_path[i]);
  }
  DEBUG_COMMENT("utilscplex.c:xstarToPath", "Path:");
  if (path != NULL) {
    DEBUG_COMMENT("utilscplex.c:xstarToPath", "Path:");
    memcpy(path, new_path, inst->nnodes * sizeof(int));
    DEBUG_COMMENT("utilscplex.c:xstarToPath", "Path:");
  } else {
    ERROR_COMMENT("utilscplex.c:xstarToPath", "path is NULL");
  }
  INSTANCE_assert(inst);
  DEBUG_COMMENT("utilscplex.c:xstarToPath", "xstarToPath ENDED");
  // if (new_path != NULL){
  //     DEBUG_COMMENT("utilscplex.c:xstarToPath", "freeing new_path");
  //     free(new_path);
  //     DEBUG_COMMENT("utilscplex.c:xstarToPath", "new_path freed");
  // }
  // new_path = NULL;
  // DEBUG_COMMENT("utilscplex.c:xstarToPath", "xstarToPath ENDED");

  free(succ);
  free(comp);
  DEBUG_COMMENT("utilscplex.c:xstarToPath", "xstarToPath ENDED");
}

void create_xheu(Instance *inst, double *xheu) {
  DEBUG_COMMENT("utilscplex.c:create_xheu", "Creating xheu");
  for (int i = 0; i < inst->nnodes - 1; i++) {
    xheu[xpos(inst->path[i], inst->path[i + 1], inst)] = 1.0;
  }
  xheu[xpos(inst->path[inst->nnodes - 1], inst->path[0], inst)] = 1.0;

  DEBUG_COMMENT("utilscplex.c:create_xheu", "xheu: ");
  for (int i = 0; i < inst->nnodes; i++) {
    for (int j = i + 1; j < inst->nnodes; j++) {
      if (xheu[xpos(i, j, inst)] > 0.5)
        DEBUG_COMMENT("utilscplex.c:create_xheu", "xheu[%d,%d] = %lf", i, j,
                      xheu[xpos(i, j, inst)]);
    }
  }
}

void generate_mip_start(Instance *inst, const CPXENVptr env, const CPXLPptr lp,
                        double *xheu) {
  INFO_COMMENT("utilscplex.c:ge*nerate_mip_start", "Generating MIP start");
  int effortlevel = CPX_MIPSTART_NOCHECK;
  int beg = 0;
  int *ind = (int *)calloc(inst->ncols, sizeof(int));
  for (int j = 0; j < inst->ncols; j++) ind[j] = j;
  if (CPXaddmipstarts(env, lp, 1, inst->ncols, &beg, ind, xheu, &effortlevel,
                      NULL))
    ERROR_COMMENT("utilscplex.c:generate_mip_start", "CPXaddmipstarts() error");
  free(ind);
  INFO_COMMENT("utilscplex.c:generate_mip_start", "MIP start ENDED");
}

void fix_edges(const CPXENVptr env, const CPXLPptr lp, Instance *inst,
               double *xheu) {
  DEBUG_COMMENT("utilscplex.c:fix_edges", "Fixing edges");
  int *ind = (int *)calloc(inst->nnodes, sizeof(int));
  double *bd = (double *)calloc(inst->nnodes, sizeof(double));
  for (int i = 0; i < inst->nnodes; i++) {
    for (int j = i + 1; j < inst->nnodes; j++) {
      if (xheu[xpos(i, j, inst)] >
          0.5)  // TODO check hyperparameter alpha 0.5 0.6 0.8
      {
        int random = rand() % 100;
        if (random < inst->percentageHF) {
          ind[i] = xpos(i, j, inst);
          bd[i] = 1.0;
          if (CPXchgbds(env, lp, 1, &ind[i], "L", &bd[i]))
            ERROR_COMMENT("utilscplex.c:fix_edges", "CPXchgbds() error");
        }
      }
    }
  }
}

void unfix_edges(const CPXENVptr env, const CPXLPptr lp, Instance *inst,
                 double *xheu) {
  INFO_COMMENT("utilscplex.c:fix_edges", "unfixing edges");
  int *ind = (int *)calloc(inst->nnodes, sizeof(int));
  double *bd = (double *)calloc(inst->nnodes, sizeof(double));
  for (int i = 0; i < inst->nnodes; i++) {
    for (int j = i + 1; j < inst->nnodes; j++) {
      ind[i] = xpos(i, j, inst);
      bd[i] = 0.0;
      if (CPXchgbds(env, lp, 1, &ind[i], "L", &bd[i]))
        ERROR_COMMENT("utilscplex.c:fix_edges", "CPXchgbds() error");
    }
  }
}

void eliminate_radius_edges(const CPXENVptr env, const CPXLPptr lp,
                            Instance *inst, double *xheu, int radious) {
  INFO_COMMENT("utilscplex.c:eliminate_fix_edges",
               "eliminating fixed edges form xheu for local branching");
  int *ind = (int *)calloc(inst->nnodes, sizeof(int));
  double *bd = (double *)calloc(inst->nnodes, sizeof(double));
  // radious should be expressed in a value between 0 and 1
  int percentage = (int)(inst->nnodes * radious);
  int count = 0;
  int flag = 0;
  while (count < percentage) {
    for (int i = 0; i < inst->nnodes; i++) {
      for (int j = 0; j < inst->nnodes; j++) {
        if (xheu[xpos(i, j, inst)] > 0.5 && (rand() % 100) < 50) {
          ind[i] = xpos(i, j, inst);
          bd[i] = 1.0;
          if (CPXchgbds(env, lp, 1, &ind[i], "L", &bd[i]))
            ERROR_COMMENT("utilscplex.c:eliminate_fix_edges",
                          "CPXchgbds() error");
          count++;
          if (count >= percentage) {
            flag = 1;
            break;
          }
        }
      }
      if (flag) break;
    }
    if (flag) break;
  }
}

void repristinate_radius_edges(const CPXENVptr env, const CPXLPptr lp,
                               Instance *inst, double *xheu) {
  INFO_COMMENT("utilscplex.c:repristinate_fix_edges",
               "repristinating fixed edges form xheu for local branching");
  int *ind = (int *)calloc(inst->nnodes, sizeof(int));
  double *bd = (double *)calloc(inst->nnodes, sizeof(double));
  for (int i = 0; i < inst->nnodes; i++) {
    for (int j = i + 1; j < inst->nnodes; j++) {
      if (xheu[xpos(i, j, inst)] > 0.5) {
        ind[i] = xpos(i, j, inst);
        bd[i] = 0.0;
        if (CPXchgbds(env, lp, 1, &ind[i], "L", &bd[i]))
          ERROR_COMMENT("utilscplex.c:repristinate_fix_edges",
                        "CPXchgbds() error");
      }
    }
  }
}

void addSubtourConstraints(CPXENVptr env, CPXLPptr lp, int nnodes,
                           int *component, int counter) {
  DEBUG_COMMENT("utilscplex.c:addSubtourConstraints",
                "Adding subtour constraints");
  for (int i = 0; i < nnodes; i++) {
    int count = 0;
    for (int j = 0; j < nnodes; j++) {
      if (component[j] == component[i]) {
        count++;
      }
    }

    if (count > 2) {  // A subtour has at least 3 nodes
      int nzcnt = 0;
      double rhs = count - 1;
      char sense = 'L';
      double rmatval[nnodes * nnodes];
      int rmatind[nnodes * nnodes];

      for (int j = 0; j < nnodes; j++) {
        for (int k = j + 1; k < nnodes; k++) {
          if (component[j] == component[i] && component[k] == component[i]) {
            rmatval[nzcnt] = 1.0;
            rmatind[nzcnt] =
                j * nnodes + k;  // Assuming x_ij variables are stored row-wise
            nzcnt++;
          }
        }
      }

      char **cname = malloc(
          sizeof(char *));  // Assuming this is enough space for constraint name
      cname[0] = malloc(30 * sizeof(char));
      sprintf(cname[0], "subtour(%d)", counter);

      int status = CPXaddrows(env, lp, 0, 1, nzcnt, &rhs, &sense, NULL, rmatind,
                              rmatval, NULL, cname);
      if (status) {
        fprintf(stderr, "Failed to add %s constraint.\n", cname[0]);
        exit(1);
      }

      counter++;  // Increment the counter for the next subtour (if any)
    }
  }
}

int bender(Instance *inst, const CPXENVptr env, const CPXLPptr lp) {
  INFO_COMMENT(
      "constraint.c:bender",
      "Adding subtour constraints, starting the while loop for BENDERS");
  int ncomp = 0;
  int n_STC = 0;
  int *succ = (int *)malloc(sizeof(int) * inst->nnodes);
  int *comp = (int *)malloc(sizeof(int) * inst->nnodes);
  double *xstar = (double *)calloc(inst->ncols, sizeof(double));
  // take the time and if exceed the time limit then break the loop
  time_t start_time = time(NULL);

  DEBUG_COMMENT("constraint.c:bender", "initialization terminated");
  while (ncomp != 1 && difftime(time(NULL), start_time) < 100) {
    DEBUG_COMMENT("constraint.c:bender", "CPXmipopt");
    if (CPXmipopt(env, lp))
      ERROR_COMMENT("constraint.c:bender",
                    "CPXmipopt() error, not able to solve the problem");

    DEBUG_COMMENT("constraint.c:bender", "CPXgetx");
    if (CPXgetx(env, lp, xstar, 0, inst->ncols - 1))
      ERROR_COMMENT("constraint.c:bender",
                    "CPXgetx() error, not able to retrive the solution");

    DEBUG_COMMENT("constraint.c:bender", "build_sol");
    build_sol(xstar, inst, succ, comp, &ncomp);
    DEBUG_COMMENT("constraint.c:bender", "ncomp = %d", ncomp);
    if (ncomp == 1) continue;

    DEBUG_COMMENT("constraint.c:bender", "addSubtourConstraints");
    addSubtourConstraints(env, lp, inst->nnodes, comp, n_STC);
  }
  free(xstar);
  free(succ);
  free(comp);
  return 0;
}

void build_sol(const double *xstar, Instance *inst, int *succ, int *comp,
               int *ncomp) {
  // INITIALIZATION
  *ncomp = 0;
  for (int i = 0; i < inst->nnodes; i++) {
    succ[i] = -1;
    comp[i] = -1;
  }

  // BUILDING THE SOLUTION
  for (int start = 0; start < inst->nnodes; start++) {
    if (comp[start] >= 0)
      continue;  // node "start" was already visited, just skip it

    // a new component is found
    (*ncomp)++;
    int i = start;
    int done = 0;
    while (!done) {
      // go and visit the current component
      comp[i] = *ncomp;
      done = 1;
      for (int j = 0; j < inst->nnodes; j++) {
        // the edge [i,j] is selected in xstar and j was not visited before
        if (i != j && xstar[xpos(i, j, inst)] > 0.5 && comp[j] == -1) {
          succ[i] = j;
          i = j;
          done = 0;
          break;
        }
      }
    }
    succ[i] = start;  // last arc to close the cycle go to the next component...
  }
}
