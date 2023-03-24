#include <stdio.h>
#include "refinement.h"

void two_opt(const double *distance_matrix, int nnodes, int *path, double *tour_length)
{
    INFO_COMMENT("refinement:2opt", "starting 2opt");
    int foundImprovement = 1;
    log_path(path, nnodes);
    double cost_old_edge, cost_new_edge, cost_new_edge2, cost_old_edge2;
    while (foundImprovement)
    {
        foundImprovement = 0;
        for (int i = 0; i < nnodes - 2; i++)
        {
            for (int j = i + 1; j < nnodes - 1; j++)
            {
                cost_old_edge = distance_matrix[path[i] * nnodes + path[i + 1]];
                cost_new_edge = distance_matrix[path[i] * nnodes + path[j]];
                cost_old_edge2 = distance_matrix[path[j] * nnodes + path[j + 1]];
                cost_new_edge2 = distance_matrix[path[i + 1] * nnodes + path[j + 1]];
                double delta = -(cost_old_edge + cost_old_edge2) + (cost_new_edge + cost_new_edge2);

                if (delta < 0)
                {
                    foundImprovement = 1;
                    int ti = i + 1;
                    for (int z = 0; z < (int)(j - i + 1) / 2; z++) // reverse order cells (i+1,index_min)
                    {
                        double temp = path[z + ti];
                        path[z + ti] = path[j - z];
                        path[j - z] = temp;
                    }
                    *tour_length += delta;
                }
            }
        }
        log_path(path, nnodes);
        DEBUG_COMMENT("refinement:2opt", "attual tour length %lf", *tour_length);
    }
}
