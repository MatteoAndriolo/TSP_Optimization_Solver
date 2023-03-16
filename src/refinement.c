#include "refinement.h"

void two_opt(double *matrix, int size_node, double *node)
{
    // 3 4 6 7 9 10 1
    for (int i = 0; i < size_node; i++)
    {
        // 1 2 3 4 5 --> cost 1_2
        double cost_edge_1 = matrix[i * size_node + i + 1];
        double min = cost_edge_1;
        int index = i + 1;
        // FIND the minmium cost distance
        for (int j = i + 1; j < size_node; j++)
        {
            // 1 2 3 4 5 --> cost 2_3 and further
            double cost_edge_2 = matrix[j * size_node + j + 1];
            if (cost_edge_2 < min)
            {
                min = cost_edge_2;
                index = j;
            }
        }

        if (index != i + 1)
        {
            for (int z = 0; z < (int)(index - i + 1) / 2; z++){
                double temp = node[z + i + 1];
                node[z + i + 1] = node[index - z];
                node[index - z] = temp;
            }  
        }
        // I and index && index_2 z
        // minmimum found at position index 4 so I want now to swap
    }