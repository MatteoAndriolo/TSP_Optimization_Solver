#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "heuristics.h"

void vnp_k(const double *distance_matrix, int *path, int nnodes, double *tour_length, int k, int duration)
{
    time_t start_time = time(NULL);
    time_t end_time = start_time + duration;
    // while non finisce il tempo
    INFO_COMMENT("heuristics.c:vnp_k", "starting the huristics loop for vnp_k");
    while (difftime(end_time, time(NULL)) > 0)
    {
        printf("Time left: %.0f seconds\n", difftime(end_time, time(NULL)));
        two_opt(distance_matrix, nnodes, path, tour_length);
        kick_function(distance_matrix, path, nnodes, tour_length, k);
    }
}

void kick_function(const double *distance_matrix, int *path, int nnodes, double *tour_length, int k)
{
    int *start = (int *)malloc(k * sizeof(int));
    //create a random number between 0 and nnodes/k
    start[0] = rand() % (nnodes / k) + 1;
    //create block size between that could be at max the number of remaining nodes divided by k
    int max_blok_size = (nnodes - start[0]) / k - 1 ;
    for (int i = 0; i < k; i++)
        start[i] = start[i - 1] + rand() % max_blok_size + rand() % ((nnodes - start[i - 1]) / k - 1);

    // genera a random number between 0 and 1
    if (rand() % 2 == 0)
    {
        //if 0 then swap the blocks form the start array
        // ABC -> BAC == (i j-1) (j k-1) (k i-1)
        // ABC -> BAC == (j k-1) (i j-1) (k i-1) to add (k-1 i) (j-1 k) to remove (j-1 j) (k-1 k) (i-1 i)
        for (int i = 0; i < k - 1; i = i + 2)
        {
            int I = start[i];
            int J = start[i + 1];
            int K = start[i + 2];
            swap_array_piece(path, I, J-1, J, K-1);
            DEBUG_COMMENT("heuristics:kick_function", "swap_array_piece(%d, %d, %d, %d)", start[i], start[i + 1] - 1, start[i + 1], start[i + 2] - 1);
            /**tour_length += distance_matrix[path[K-1] * nnodes + path[I]]
                          + distance_matrix[path[J-1] * nnodes + path[K]]
                          + distance_matrix[path[I-1] * nnodes + path[J]]
                          - distance_matrix[path[J-1] * nnodes + path[J]]
                          - distance_matrix[path[K-1] * nnodes + path[K]]
                          - distance_matrix[path[I-1] * nnodes + path[I]];*/
        }
    }else{
        swap_array_piece(path, start[k], nnodes - 1, start[k-1], start[k-1] + nnodes - start[k] - 1);
        DEBUG_COMMENT("heuristics:kick_function", "swap_array_piece(%d, %d, %d, %d)", start[k], nnodes - 1, start[k - 1], start[k - 1] + nnodes - start[k] - 1);
        swap_array_piece(path, start[0], start[1] - 1, start[k-1] + nnodes - start[k], start[k] - 1);
        DEBUG_COMMENT("heuristics:kick_function", "swap_array_piece(%d, %d, %d, %d)", start[0], start[1] - 1, start[k - 1] + nnodes - start[k], start[k] - 1);
        for (int i = k - 2; i >= 0; i = i - 2){
            int I = start[i];
            int J = start[i-1];
            int K = start[i-2];
            swap_array_piece(path, I-1, K, K-1, J-1);
        DEBUG_COMMENT("heuristics:kick_function", "swap_array_piece(%d, %d, %d, %d)", start[i] - 1, start[i - 2], start[i - 1] - 1, start[i - 1]);
        }
    }

    double new_path_cost = 0;
    for (int i = 0; i < nnodes - 1; i++)
    {
        new_path_cost += distance_matrix[path[i] * nnodes + path[i + 1]];
    }
    new_path_cost += distance_matrix[path[nnodes - 1] * nnodes + path[0]];
    *tour_length = new_path_cost;  
}