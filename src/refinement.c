#include "refinement.h"

void two_opt(double *distance_matrix, int nnode, int *path)
{
    // 3 4 6 7 9 10 1
    for (int i = 0; i < nnode; i++)
    {
        // SET BASELINE
        double min_cost = distance_matrix[path[i] * nnode + path[i + 1]];
        int index_min = i + 1;
        // FIND THE MINIMUM COST EDGE
        double cost_edge;
        for (int j = index_min; j < nnode; j++)
        {
            cost_edge = distance_matrix[path[i] * nnode + path[j]]; 
            if (cost_edge < min_cost)
            {
                min_cost = cost_edge;
                index_min = j;
            }
        }

        if (index_min != i + 1) // found new optimal edge (i, index_min)
        {
            int ti=i+1;
            for (int z = 0; z < (int)(index_min - i + 1) / 2; z++) // reverse order cells (i+1,index_min)
            {
                double temp = path[z + ti];
                path[z + ti] = path[index_min - z];
                path[index_min - z] = temp;
            }
        }
    }
}
