#include "../include/refinement.h"

#include <stdio.h>

#include "../include/metaheuristic.h"
#include "../include/tabu.h"
#include "../include/utils.h"

ErrorCode _two_opt(Instance *inst, double iterations, bool update) {
  INFO_COMMENT("refinement.c:2opt", "starting 2opt");
  int foundImprovement = 1;
  double cost_old_edge, cost_new_edge, cost_new_edge2, cost_old_edge2;
  double c_iter = 0;
  while (c_iter < iterations && foundImprovement) {
    foundImprovement = 0;
    for (int i = 0; i < inst->nnodes - 2; i++) {
      for (int j = i + 1; j < inst->nnodes; j++) {
        int j1 = (j + 1) % inst->nnodes;
        cost_old_edge = INSTANCE_getDistancePos(inst, i, i + 1);
        cost_new_edge = INSTANCE_getDistancePos(inst, i, j);
        cost_old_edge2 = INSTANCE_getDistancePos(inst, j, j1);
        cost_new_edge2 = INSTANCE_getDistancePos(inst, i + 1, j1);
        double delta = -(cost_old_edge + cost_old_edge2) +
                       (cost_new_edge + cost_new_edge2);

        if (delta < 0) {
          foundImprovement = 1;
          two_opt_move(inst, i, j);
          if (update) {
            RUN(INSTANCE_pathCheckpoint(inst));
          }
        }
        // DEBUG_COMMENT("refinement.c:2opt", "delta %lf, tl %lf", delta,
        // tour_length);
        // CHECKTIME(inst, false);
      }
      c_iter++;
      if (c_iter > iterations) {
        foundImprovement = false;
        break;
      }
    }
  }
  DEBUG_COMMENT("refinement.c:2opt", "attual tour length %lf",
                inst->tour_length);
  return SUCCESS;
}
ErrorCode two_opt(Instance *inst, double iterations) {
  return _two_opt(inst, iterations, true);
}

ErrorCode two_opt_noupdate(Instance *inst, double iterations) {
  return _two_opt(inst, iterations, false);
}

ErrorCode tabu_search(Instance *inst, double iterations) {
  TabuList *tabu = initializeTabuList(inst->nnodes / 5, 4);

  for (int i = 0; i < iterations; i++) {
    kick_function_tabu(inst, (int)(inst->nnodes / 10), tabu);
    two_opt_tabu(inst, INFINITY, tabu);
    RUN(INSTANCE_pathCheckpoint(inst));
  }
  return SUCCESS;
}

ErrorCode two_opt_tabu(Instance *inst, double iterations, TabuList *tabuList) {
  INFO_COMMENT("refinement.c:2opt_tabu", "starting 2opt");
  bool foundImprovement = true;
  double cost_old_edge, cost_new_edge, cost_new_edge2, cost_old_edge2;
  double c_iter = 0;
  while (c_iter < iterations && foundImprovement) {
    foundImprovement = false;

    for (int i = 0; i < inst->nnodes - 2; i++) {
      c_iter++;
      for (int j = i + 1; j < inst->nnodes; j++) {
        // get distances
        int j1 = (j + 1) % inst->nnodes;
        cost_old_edge = INSTANCE_getDistancePos(inst, i, i + 1);
        cost_new_edge = INSTANCE_getDistancePos(inst, i, j);
        cost_old_edge2 = INSTANCE_getDistancePos(inst, j, j1);
        cost_new_edge2 = INSTANCE_getDistancePos(inst, i + 1, j1);
        double delta = -(cost_old_edge + cost_old_edge2) +
                       (cost_new_edge + cost_new_edge2);

        if (delta < 0 &&
            !areAnyValuesTabu(tabuList, inst->path[i], inst->path[j],
                              inst->path[i + 1], inst->path[j1])) {
          two_opt_move(inst, i, j);

          incAgeTabuList(tabuList);
          addTabu(tabuList, i);

          foundImprovement = true;
          break;
        }
      }
    }
    CHECKTIME(inst, false);
  }

  INFO_COMMENT("refinement.c:2opt_tabu", "finished - final tour length %lf",
               inst->tour_length);
  return SUCCESS;
}
