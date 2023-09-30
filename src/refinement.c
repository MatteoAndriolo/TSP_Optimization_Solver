#include <stdio.h>

#include "../include/refinement.h"
#include "../include/heuristics.h"
#include "../include/tabu.h"

void two_opt(Instance *inst, double iterations) {
    INFO_COMMENT("refinement:2opt", "starting 2opt");
    int foundImprovement = 1;
    double cost_old_edge, cost_new_edge, cost_new_edge2, cost_old_edge2;
    double c_iter = 0;
    while (c_iter < iterations && foundImprovement) {
        foundImprovement = 0;
        for (int i = 0; i < inst->nnodes - 2; i++) {
            for (int j = i + 1; j < inst->nnodes; j++) {
                int j1 = (j + 1) % inst->nnodes;
                cost_old_edge = getDistancePos(inst,i, i+1);
                cost_new_edge = getDistancePos(inst, i, j);
                cost_old_edge2 = getDistancePos(inst,j, j1);
                cost_new_edge2 = getDistancePos(inst, i+1, j1);
                double delta = -(cost_old_edge + cost_old_edge2) +
                    (cost_new_edge + cost_new_edge2);

                if (delta < 0) {
                    foundImprovement = 1;
                    two_opt_move(inst, i, j);
                }
                c_iter++;
                if (c_iter > iterations) return;
                // DEBUG_COMMENT("refinement:2opt", "delta %lf, tl %lf", delta,
                // tour_length);
            }
        }
        if (time(NULL) > inst->max_time) break;
    }
    calculateTourLength(inst);
    DEBUG_COMMENT("refinement:2opt", "attual tour length %lf", inst->tour_length);
}

void two_opt_tabu(Instance *inst,  double iterations, TabuList *tabuList) {
    INFO_COMMENT("refinement:2opt_tabu", "starting 2opt");
    int foundImprovement = 1;
    double cost_old_edge, cost_new_edge, cost_new_edge2, cost_old_edge2;
    double c_iter = 0;
    while (c_iter < iterations && foundImprovement) {
        foundImprovement = 0;
        for (int i = 0; i < inst->nnodes - 2; i++) {
            for (int j = i + 1; j < inst->nnodes; j++) {
                int j1 = (j + 1) % inst->nnodes;
                cost_old_edge = getDistancePos(inst,i, i+1);
                cost_new_edge = getDistancePos(inst, i, j);
                cost_old_edge2 = getDistancePos(inst,j, j1);
                cost_new_edge2 = getDistancePos(inst, i+1, j1);
                double delta = -(cost_old_edge + cost_old_edge2) +
                    (cost_new_edge + cost_new_edge2);
                if (delta < 0 && areAnyValuesTabu(tabuList, inst->path[i], inst->path[j], inst->path[i+1],inst->path[j1]))
                    {
                    foundImprovement = 1;
                    two_opt_move(inst, i, j);
                    DEBUG_COMMENT("refinement:2opt_tabu", "two opt movement %d %d", i, j);
                    if (!simpleCorrectness(inst->path, inst->nnodes)) {
                        FATAL_COMMENT("refinement:2opt_tabu", "simple correctness failed")
                    }
                }
                c_iter++;
            }
        }
    }
    // assert_path(inst->path, distance_matrix, inst->nnodes, *tour_length);
    calculateTourLength(inst);
    INFO_COMMENT("refinement:2opt_tabu", "finished - final tour length %lf",
            inst->tour_length);
}
