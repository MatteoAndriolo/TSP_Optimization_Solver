#include <stdio.h>
#include "refinement.h"

void two_opt(const double *distance_matrix, int nnodes, int *path, double *tour_length, double iterations)
{
    INFO_COMMENT("refinement:2opt", "starting 2opt");
    int foundImprovement = 1;
    //log_path(path, nnodes);
    double cost_old_edge, cost_new_edge, cost_new_edge2, cost_old_edge2;
    double c_iter=0;
    while (c_iter<iterations && foundImprovement)
    {
        foundImprovement = 0;
        for (int i = 0; i < nnodes - 2; i++)
        {
            for (int j = i + 1; j < nnodes ; j++)
            {
                int j1=(j+1)%nnodes;
                cost_old_edge = distance_matrix[path[i] * nnodes + path[i + 1]];
                cost_new_edge = distance_matrix[path[i] * nnodes + path[j]];
                cost_old_edge2 = distance_matrix[path[j] * nnodes + path[j1]];
                cost_new_edge2 = distance_matrix[path[i + 1] * nnodes + path[j1]];
                double delta = -(cost_old_edge + cost_old_edge2) + (cost_new_edge + cost_new_edge2);

                if (delta < 0)
                {
                    foundImprovement = 1;
                    two_opt_move(path, i,j,nnodes);
                }
                c_iter++;
                if(c_iter>iterations)
                    return;
                //DEBUG_COMMENT("refinement:2opt", "delta %lf, tl %lf", delta, tour_length);
            }
        }
    }
    //log_path(path, nnodes);
    DEBUG_COMMENT("refinement:2opt", "attual tour length %lf", *tour_length);
}
