#include "../include/refinement.h"

#include "../include/tabu.h"
#include "../include/utils.h"

ErrorCode two_opt(Instance *inst, double iterations) {
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
  RUN(INSTANCE_saveBestPath(inst));
  DEBUG_COMMENT("refinement.c:2opt", "attual tour length %lf",
                inst->tour_length);
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
          INSTANCE_calculateTourLength(inst);

          incAgeTabuList(tabuList);
          addTabu(tabuList, i);

          foundImprovement = true;
          break;
        }
      }
    }
    CHECKTIME(inst, false);
  }

  RUN(INSTANCE_saveBestPath(inst));
  INFO_COMMENT("refinement.c:2opt_tabu", "finished - final tour length %lf",
               inst->tour_length);
  return SUCCESS;
}
