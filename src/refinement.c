#include "refinement.h"

void two_opt(double *distance_matrix, int nnode, int *path, double *tour_lenght)
{
    INFO_COMMENT("refinement:two_opt", "Starting 2-opt refinement");
    double improvement = -INFINITY;
    int index_min = -1;
    double min_cost = -1;
    double count = 0;
    /*- count è un contatore che ha come logica quella di fare  check se non ci sono stati un tot di improvement in 2 optù
      - ho aggiunto nella condizione if il fatto che se improvement è >0 non deve fare nulla e aumenta il count 
      - se alla fine della fiera non trova abbastanza improvement taglia il while e finisce 2opt*/
    while (count < ((int)nnode * 0.4))
    {
        count = 0;
        for (int i = 0; i < nnode; i++)
        {
            index_min = i + 1;
            // SET BASELINE
            min_cost = distance_matrix[path[i] * nnode + path[i + 1]];
           
            // FIND THE MINIMUM COST EDGE
            double cost_edge;
            for (int j = index_min; j < nnode; j++)
            {
                cost_edge = distance_matrix[path[i] * nnode + path[j]];
                if (cost_edge < min_cost)
                {
                    min_cost = cost_edge;
                    index_min = j;
                    DEBUG_COMMENT("refinement:two_opt", "min_cost: %f", min_cost);
                    DEBUG_COMMENT("refinement:two_opt", "index_min: %d", index_min);
                }
            }
            improvement =   distance_matrix[path[i] * nnode + path[index_min]]                // ADD THE SMALL ONE FOUND
                          - distance_matrix[path[i] * nnode + path[i + 1]]                  // ELIMINATE OLD ONE
                          + distance_matrix[path[i + 1] * nnode + path[index_min + 1]]      // ADD LINK BETWEEN i+1 and min_index + 1
                          - distance_matrix[path[index_min] * nnode + path[index_min + 1]]; // ELIMINATE index_min and the position after min_index
            // ADD THE BASELINE WITH THE MIN FOUND SUCCESSOR
            *tour_lenght += improvement;
            CRITICAL_COMMENT("refinement:two_opt", "Improvement: %f", improvement);
            CRITICAL_COMMENT("refinement:two_opt", "add i + min_index: %f", min_cost);
            CRITICAL_COMMENT("refinement:two_opt", "delated min_index and min_index + 1: %f", distance_matrix[path[index_min] * nnode + path[index_min + 1]]);
            CRITICAL_COMMENT("refinement:two_opt", "delated i and i + 1 : %f", distance_matrix[path[i] * nnode + path[i + 1]]);
            CRITICAL_COMMENT("refinement:two_opt", "add i + 1 and min_index + 1: %f", distance_matrix[path[i + 1] * nnode + path[index_min + 1]]);
            if (index_min != i + 1 && improvement < 0) // found new optimal edge (i, index_min)
            {
                int ti = i + 1;
                for (int z = 0; z < (int)(index_min - i + 1) / 2; z++) // reverse order cells (i+1,index_min)
                {
                    double temp = path[z + ti];
                    path[z + ti] = path[index_min - z];
                    path[index_min - z] = temp;
                }
            }else count++;
        }
    }
}
