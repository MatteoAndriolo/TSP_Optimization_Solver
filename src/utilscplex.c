#include "../include/utilscplex.h"

#include <stdio.h>
#include <string.h>

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
void init_mip(const CPXENVptr env, const CPXLPptr lp, Instance *inst) {
  double *xheu = (double *)calloc(inst->ncols, sizeof(double));
  create_xheu(inst, xheu);
  set_mip_start(inst, env, lp, xheu);
  free(xheu);
  xheu = NULL;
}
////////////////////

int xpos(int i, int j, Instance *inst) {
  if (i == j)
    ERROR_COMMENT("utilscplex.c:xpos", "i == j");
  if (i > j)
    return xpos(j, i, inst);
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

ErrorCode xstarToPath(Instance *inst, const double *xstar, int dim_xstar,
                      int *path) {
  DEBUG_COMMENT("utilscplex.c:xstarToPath", "xstarToPath");

  dim_xstar = inst->ncols;
  // double *temp_path = malloc(inst->nnodes * 2 * sizeof(double));
  double temp_path[inst->ncols];
  memcpy(temp_path, xstar, inst->ncols * sizeof(double));
  qsort(temp_path, inst->ncols, sizeof(int), cmpfunc);
  // DEBUG_COMMENT("utilscplex.c:xstarToPath", "Path: ");
  // int count = 0;
  // for (int i = 0; i < dim_xstar; i++) {
  //   if (temp_path[i] > 0.5) {
  //     count++;
  //     DEBUG_COMMENT("utilscplex.c:xstarToPath", "%d/%d | %d -> %lf ", i,
  //                   dim_xstar, count, temp_path[i]);
  //   }
  // }
  // free(temp_path);
  // temp_path = NULL;

  int succ[inst->nnodes];
  int comp[inst->nnodes];
  // int *succ = (int *)malloc(sizeof(int) * inst->nnodes);
  // int *comp = (int *)malloc(sizeof(int) * inst->nnodes);
  int ncomp = 0;

  build_sol(xstar, inst, succ, comp, &ncomp);

  // DEBUG_COMMENT("utilscplex.c:xstarToPath", "ncomp = %d", ncomp);
  // for (int i = 0; i < inst->nnodes; i++) {
  //   DEBUG_COMMENT("utilscplex.c:xstarToPath", "%d\t%d\t%d", i, succ[i],
  //                 comp[i]);
  // }

  // int *new_path = (int *)malloc(sizeof(int) * inst->nnodes);
  int new_path[inst->nnodes];
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

  // for (int i = 0; i < inst->nnodes; i++) {
  //   DEBUG_COMMENT("utilscplex.c:xstarToPath", "%d -> %d", i, new_path[i]);
  // }
  if (path != NULL) {
    memcpy(path, new_path, inst->nnodes * sizeof(int));
  } else {
    ERROR_COMMENT("utilscplex.c:xstarToPath", "path is NULL");
  }
  ASSERTINST(inst);
  DEBUG_COMMENT("utilscplex.c:xstarToPath", "xstarToPath ENDED");
}

void create_xheu(Instance *inst, double *xheu) {
  DEBUG_COMMENT("utilscplex.c:create_xheu", "Creating xheu");
  for (int i = 0; i < inst->nnodes - 1; i++) {
    xheu[xpos(inst->path[i], inst->path[i + 1], inst)] = 1.0;
  }
  xheu[xpos(inst->path[inst->nnodes - 1], inst->path[0], inst)] = 1.0;

  // DEBUG_COMMENT("utilscplex.c:create_xheu", "xheu: ");
  // for (int i = 0; i < inst->nnodes; i++) {
  //   for (int j = i + 1; j < inst->nnodes; j++) {
  //     if (xheu[xpos(i, j, inst)] > 0.5)
  //       DEBUG_COMMENT("utilscplex.c:create_xheu", "xheu[%d,%d] = %lf", i, j,
  //                     xheu[xpos(i, j, inst)]);
  //   }
  // }
}

void set_mip_start(Instance *inst, const CPXENVptr env, const CPXLPptr lp,
                   double *xheu) {
  INFO_COMMENT("utilscplex.c:ge*nerate_mip_start", "Generating MIP start");
  int effortlevel = CPX_MIPSTART_NOCHECK;
  int beg = 0;
  int *ind = (int *)calloc(inst->ncols, sizeof(int));
  for (int j = 0; j < inst->ncols; j++)
    ind[j] = j;
  if (CPXaddmipstarts(env, lp, 1, inst->ncols, &beg, ind, xheu, &effortlevel,
                      NULL))
    ERROR_COMMENT("utilscplex.c:generate_mip_start", "CPXaddmipstarts() error");
  free(ind);
  INFO_COMMENT("utilscplex.c:set_mip_start", "MIP start ENDED");
}

void fix_edges(const CPXENVptr env, const CPXLPptr lp, Instance *inst,
               double *xheu) {
  DEBUG_COMMENT("utilscplex.c:fix_edges", "Fixing edges");
  int *ind = (int *)calloc(inst->nnodes, sizeof(int));
  double *bd = (double *)calloc(inst->nnodes, sizeof(double));
  for (int i = 0; i < inst->nnodes; i++) {
    for (int j = i + 1; j < inst->nnodes; j++) {
      if (xheu[xpos(i, j, inst)] >
          0.5) // TODO check hyperparameter alpha 0.5 0.6 0.8
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
      if (flag)
        break;
    }
    if (flag)
      break;
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

bool isConstraintNotValidForCurrOpt(Instance *inst, double *xstar, int *rmatind,
                                    int nzcnt, int rhs) {
  int sum = 1;
  for (int i = 0; i < nzcnt; i++) {
    sum += xstar[rmatind[i]];
  }
  INFO_COMMENT("utilscplex.c:isConstraintNotValidForCurrOpt",
               "sum = %d, rhs = %d", sum, rhs);
  if (sum <= rhs) {
    return false;
  }
  return true;
}

int addSubtourConstraints(CPXENVptr env, CPXLPptr lp, int nnodes, int *comp,
                          Instance *inst, int *counter, double *xstar) {
  DEBUG_COMMENT("utilscplex.c:addSubtourConstraints",
                "Adding subtour constraints");
  // INIT
  int *component = malloc(nnodes * sizeof(int));
  memcpy(component, comp, nnodes * sizeof(int));
  int nodeSubTour[nnodes];
  int nNodeSubTour = 0;
  // Allocate memory for the double array
  double rmatval[inst->ncols];
  for (size_t i = 0; i < inst->ncols; i++) {
    rmatval[i] = 1.0;
  }

  // SCAN ALL NODES
  for (int i = 0; i < nnodes; i++) {
    nNodeSubTour = 0;
    if (component[i] < 0)
      continue; // node "i" was already visited, just skip it
    int current_component = component[i];
    nodeSubTour[nNodeSubTour++] = i;
    component[i] = -1; // mark node "i" as visited

    // LOOK WITHING COMPONENT
    for (int j = 0; j < nnodes; j++) {
      if (component[j] == current_component) {
        component[j] = -1; // mark node "j" as visited
        nodeSubTour[nNodeSubTour] = j;
        nNodeSubTour++;
      }
    }

    INFO_COMMENT("utilscplex.c:addSubtourConstraints",
                 "current component has %d nodes", nNodeSubTour);

    if (nNodeSubTour > 2) { // A subtour has at least 3 nodes
      // PARAM FOR CPXaddrows
      int ccnt = 0;
      int rcnt = 1;
      int nzcnt = 0;
      double rhs = nNodeSubTour - 1;
      char sense = 'L';
      int rmatbeg[] = {0};
      // double rmatval = {all 1};

      int rmatind[inst->ncols];
      for (int j = 0; j < nNodeSubTour; j++)
        for (int k = j + 1; k < nNodeSubTour; k++)
          rmatind[nzcnt++] = xpos(nodeSubTour[j], nodeSubTour[k], inst);

      char **cname = malloc(sizeof(char *));
      cname[0] = malloc(30 * sizeof(char));
      sprintf(cname[0], "subtour(%d)", *counter);

      // debug comment number of nzcnt
      DEBUG_COMMENT("utilscplex.c:addSubtourConstraints", "nzcnt = %d", nzcnt);
      // debug list of rmatval != 1

      if (!isConstraintNotValidForCurrOpt(inst, xstar, rmatind, nzcnt, rhs)) {
        ERROR_COMMENT("utilscplex.c:addSubtourConstraints",
                      "constraint valid for current opt");
        return ERROR_NODES;
      }
      int status = CPXaddrows(env, lp, ccnt, rcnt, nzcnt, &rhs, &sense, rmatbeg,
                              rmatind, rmatval, NULL, cname);
      if (status) {
        ERROR_COMMENT("utilscplex.c:addSubtourConstraints",
                      "CPXaddrows(): error 1");
        return status;
      }
      DEBUG_COMMENT("utilscplex.c:addSubtourConstraints",
                    "constraint added, number rows is %d",
                    CPXgetnumrows(env, lp));

      (*counter)++; // Increment the counter for the next subtour (if any)
      free(cname[0]);
      free(cname);
    }
  }
  free(component);
  component = NULL;
  return SUCCESS;
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
      continue; // node "start" was already visited, just skip it

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
    succ[i] = start; // last arc to close the cycle go to the next component...
  }
}
void build_model(Instance *inst, const CPXENVptr env, const CPXLPptr lp) {
  INFO_COMMENT("utilscplex.c:build_model", "Building model");

  char **cname =
      (char **)malloc(inst->nnodes * (inst->nnodes - 1) / 2 *
                      sizeof(char *)); // (char **) required by cplex...
  if (!cname)
    ERROR_COMMENT("utilscplex.c:build_model", "wrong allocation of cname");

  inst->ncols = inst->nnodes * (inst->nnodes - 1) / 2;
  inst->edgeList =
      (double *)calloc((inst->nnodes * (inst->nnodes - 1)) / 2, sizeof(double));
  double *obj = (double *)calloc(inst->ncols, sizeof(double));
  double *lb = (double *)calloc(inst->ncols, sizeof(double));
  double *ub = (double *)calloc(inst->ncols, sizeof(double));
  char *xctype = (char *)calloc(inst->ncols, sizeof(char));

  int c = 0;
  for (int i = 0; i < inst->nnodes - 1; i++) {
    for (int j = i + 1; j < inst->nnodes; j++) {
      cname[c] = (char *)calloc(20, sizeof(char));
      sprintf(cname[c], "x(%d,%d)", i + 1,
              j + 1); // (i+1,j+1) because CPLEX starts from 1 (not 0)
      obj[c++] = INSTANCE_getDistanceNodes(inst, i, j);
    }
  }
  if (c != inst->ncols)
    ERROR_COMMENT("utilscplex.c:build_model", "wrong number of columns");
  for (int i = 0; i < inst->ncols; i++) {
    // lb[i] = 0.0;
    ub[i] = 1.0;
    xctype[i] = CPX_BINARY;
  }

  int status = CPXnewcols(env, lp, inst->ncols, obj, lb, ub, xctype, cname);
  DEBUG_COMMENT("tspcplex.c:build_model", "status %d", status);
  free(obj);
  free(lb);
  free(ub);
  free(xctype);
  DEBUG_COMMENT("utilscplex.c:build_model",
                "number of columns in CPLEX after adding x var.s %d",
                CPXgetnumcols(env, lp));

  if (inst->ncols != CPXgetnumcols(env, lp))
    ERROR_COMMENT(
        "utilscplex.c:build_model",
        " wrong number of columns in CPLEX after adding x var.s %d vs %d",
        inst->ncols, CPXgetnumcols(env, lp));
  if (inst->ncols != (inst->nnodes * (inst->nnodes - 1)) / 2)
    ERROR_COMMENT("utilscplex.c:build_model",
                  "wrong number of columns in CPLEX after adding x var.s");

  // ADD CONSTRAITS - ROWS
  // add_degree_constraint(inst, env, lp) for each node
  INFO_COMMENT("constraint.c:add_degree_constraint",
               "Adding degree constraints");
  int *index = (int *)calloc(inst->nnodes, sizeof(int));
  double *value = (double *)calloc(inst->nnodes, sizeof(double));

  for (int i = 0; i < inst->nnodes; i++) {
    double rhs = 2.0;
    char sense = 'E';                       // 'E' for equality constraint
    sprintf(cname[0], "degree(%d)", i + 1); // rowname
    int nnz = 0; // number of non zero constraints to be added to constraint
                 // matrix length of array rmatind rmatval
    for (int j = 0; j < inst->nnodes; j++) {
      if (j == i)
        continue;
      index[nnz] = xpos(j, i, inst); // rmatind
      value[nnz] = 1.0;              // rmatval
      nnz++;
    }
    int izero = 0; // rmatbeg
                   // int  CPXaddrows( CPXCENVptr env, CPXLPptr lp, int ccnt,
                   // int rcnt, int nzcnt, double const * rhs, char const *
                   // sense, int const * rmatbeg, int const * rmatind, double
                   // const * rmatval, char ** colname, char ** rowname
                   // )
    if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL,
                   &cname[0]))
      ERROR_COMMENT("constraint.c:add_degree_constraint",
                    "CPXaddrows(): error 1");
  }
  DEBUG_COMMENT("constraint.c:add_degree_constraint",
                "NUMBER OF ROW IN CPLEX after adding degree constraints %d",
                CPXgetnumrows(env, lp));

  // FREE
  for (int i = 0; i < inst->nnodes * (inst->nnodes - 1) / 2; i++)
    free(cname[i]);
  free(cname);
  free(index);
  free(value);
}

int patchPath(Instance *inst, double *xstar, int *succ, int *comp, int *path,
              double *obj_value) {
  DEBUG_COMMENT("utilscplex.c:patchPath", "Patching path");
  xstarToPath(inst, xstar, inst->ncols, path);
  int component[inst->nnodes];
  memcpy(component, comp, inst->nnodes * sizeof(int));
  int counter = 0;
  for (int i = 0; i < inst->nnodes; i++) {
    if (component[i] < 0)
      continue;
    int current_component = component[i];
    path[counter++] = i;
    component[i] = -1;

    int succ_node = succ[i];
    while (component[succ_node] == current_component) {
      path[counter++] = succ_node;
      component[succ_node] = -1;
      succ_node = succ[succ_node];
    }
  }
  if (counter != inst->nnodes) {
    ERROR_COMMENT("utilscplex.c:patchPath", "counter != inst->nnodes");
    return ERROR;
  }

  return SUCCESS;
}
