#include "refinement.h"
/*
void two_opt(double *distance_matrix, int nnodes, int *path, double *tour_length)
{
    INFO_COMMENT("refinement:two_opt", "Starting 2-opt refinement");
    double init_tourLength=*tour_length;
    int index_min;
    for (int c = 0; c < 10; c++)
    {
        double improvement = 0;
        DEBUG_COMMENT("refinement:two_opt", "start iteration %d", c);
        
        for (int i = 0; i < nnodes; i++)
        {
            index_min = i + 1;

            // FIND THE MINIMUM COST EDGE
            double cost_old_edge, cost_new_edge, cost_new_edge2, cost_old_edge2, timprovment;
            for (int j = index_min; j < nnodes; j++)
            {
                cost_old_edge = distance_matrix[path[i] * nnodes + path[i + 1]];
                cost_new_edge = distance_matrix[path[i] * nnodes + path[j]];
                cost_old_edge2 = distance_matrix[path[j] * nnodes + path[j + 1]];
                cost_new_edge2 = distance_matrix[path[i + 1] * nnodes + path[j + 1]];
                timprovment = (cost_old_edge + cost_old_edge2) - (cost_new_edge + cost_new_edge2);

                if (cost_new_edge < cost_old_edge && cost_new_edge2 < cost_old_edge2)
                {
                    // min_cost = cost_new_edge;
                    DEBUG_COMMENT("refinement:two_opt", "Improvement: %lf %lf %lf %lf %lf", cost_old_edge, cost_new_edge, cost_old_edge2, cost_new_edge2, timprovment);
                    index_min = j;
                    improvement = timprovment;
                }
            }

            // ADD THE BASELINE WITH THE MIN FOUND SUCCESSOR
            if (index_min != i + 1 && improvement > 0) // found new optimal edge (i, index_min)
            {
                int ti = i + 1;
                for (int z = 0; z < (int)(index_min - i + 1) / 2; z++) // reverse order cells (i+1,index_min)
                {
                    double temp = path[z + ti];
                    path[z + ti] = path[index_min - z];
                    path[index_min - z] = temp;
                }
                CRITICAL_COMMENT("refinement:two_opt", "Improvement: %f", improvement);
                *tour_length += improvement;
            }
        }
        DEBUG_COMMENT("refinement:2opt","iteration %d, improvment %lf", c, init_tourLength-*tour_length);
        init_tourLength=*tour_length;
        log_output(2, path[0], *tour_length, 0, 23, nnodes, "file");
        log_path(path, nnodes);
    }
}
*/


void two_opt(double *distance_matrix, int nnodes, int *path, double *tour_length ){
    int foundImprovement = 1;
    log_path(path, nnodes);
    double cost_old_edge, cost_new_edge, cost_new_edge2, cost_old_edge2;
    while(foundImprovement){
        foundImprovement = 0;
        for(int i=0; i< nnodes-1; i++){
            for(int j=i+1; j<nnodes;j++){
                cost_old_edge = distance_matrix[path[i] * nnodes + path[i + 1]];
                cost_new_edge = distance_matrix[path[i] * nnodes + path[j]];
                cost_old_edge2 = distance_matrix[path[j] * nnodes + path[j + 1]];
                cost_new_edge2 = distance_matrix[path[i + 1] * nnodes + path[j + 1]];
                double delta = -(cost_old_edge + cost_old_edge2) + (cost_new_edge + cost_new_edge2);

                if (delta <0){
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
        DEBUG_COMMENT("refinement:2opt","attual tour length %lf", *tour_length);
    }
}
