#include <stdio.h>
#include "refinement.h"
#include "heuristics.h"

void two_opt(const double *distance_matrix, int nnodes, int *path, double *tour_length, double iterations)
{
    INFO_COMMENT("refinement:2opt", "starting 2opt");
    int foundImprovement = 1;
    double cost_old_edge, cost_new_edge, cost_new_edge2, cost_old_edge2;
    double c_iter = 0;
    while (c_iter < iterations && foundImprovement)
    {
        foundImprovement = 0;
        for (int i = 0; i < nnodes - 2; i++)
        {
            for (int j = i + 1; j < nnodes; j++)
            {
                int j1 = (j + 1) % nnodes;
                cost_old_edge = distance_matrix[path[i] * nnodes + path[i + 1]];
                cost_new_edge = distance_matrix[path[i] * nnodes + path[j]];
                cost_old_edge2 = distance_matrix[path[j] * nnodes + path[j1]];
                cost_new_edge2 = distance_matrix[path[i + 1] * nnodes + path[j1]];
                double delta = -(cost_old_edge + cost_old_edge2) + (cost_new_edge + cost_new_edge2);

                if (delta < 0)
                {
                    foundImprovement = 1;
                    two_opt_move(path, i, j, nnodes);
                    DEBUG_COMMENT("refinement:two_opt", "2opt move: n1=%d, n2=%d, delta=%lf", i, j, delta);
                }
                c_iter++;
                if (c_iter > iterations)
                    return;
            }
        }
    }
    *tour_length = get_tour_length(path, nnodes, distance_matrix);
    DEBUG_COMMENT("refinement:2opt", "attual tour length %lf", *tour_length);
}

void two_opt_tabu(const double *distance_matrix, int nnodes, int *path, double *tour_length, double iterations, CircularBuffer *tabuList)
{
    INFO_COMMENT("refinement:2opt_tabu", "starting 2opt");
    int foundImprovement = 1;
    double cost_old_edge, cost_new_edge, cost_new_edge2, cost_old_edge2;
    double c_iter = 0;
    while (c_iter < iterations && foundImprovement)
    {
        foundImprovement = 0;
        for (int i = 0; i < nnodes - 2; i++)
        {
            for (int j = i + 1; j < nnodes; j++)
            {
                int j1 = (j + 1) % nnodes;
                cost_old_edge = distance_matrix[path[i] * nnodes + path[i + 1]];
                cost_new_edge = distance_matrix[path[i] * nnodes + path[j]];
                cost_old_edge2 = distance_matrix[path[j] * nnodes + path[j1]];
                cost_new_edge2 = distance_matrix[path[i + 1] * nnodes + path[j1]];
                double delta = -(cost_old_edge + cost_old_edge2) + (cost_new_edge + cost_new_edge2);
                if (delta < 0 && !(contains(tabuList, path[i]) || contains(tabuList, path[j]) || contains(tabuList, path[i + 1]) || contains(tabuList, path[j1])))
                {
                    foundImprovement = 1;
                    two_opt_move(path, i, j, nnodes);
                    DEBUG_COMMENT("refinement:2opt_tabu", "two opt movement %d %d", i, j);
                    if (!simpleCorrectness(path, nnodes))
                    {
                        FATAL_COMMENT("refinement:2opt_tabu", "simple correctness failed")
                    }
                }
                c_iter++;
            }
        }
    }
    // assert_path(path, distance_matrix, nnodes, *tour_length);
    *tour_length = get_tour_length(path, nnodes, distance_matrix);
    INFO_COMMENT("refinement:2opt_tabu", "finished - final tour length %lf", *tour_length);
}
