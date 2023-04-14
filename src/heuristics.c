#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "heuristics.h"

void vnp_k(const double *distance_matrix, int *path, int nnodes, double *tour_length, int k, int duration)
{
    time_t start_time = time(NULL);
    time_t end_time = start_time + duration;
    double best_path = INFINITY;
    int *best_node_tour = (int *)malloc(nnodes * sizeof(int));
    // while non finisce il tempo
    INFO_COMMENT("heuristics.c:vnp_k", "starting the huristics loop for vnp_k");
    while (difftime(end_time, time(NULL)) > 0)
    {
        printf("Time left: %.0f seconds\n", difftime(end_time, time(NULL)));
        DEBUG_COMMENT("heuristics.c:vnp_k", "best passed for the TOWOPT function = %f", best_path);
        two_opt(distance_matrix, nnodes, path, tour_length);
        if (best_path < *tour_length)
        {
            best_path = *tour_length;
            memcpy(best_node_tour, path, nnodes * sizeof(int));
        }
        DEBUG_COMMENT("heuristics.c:vnp_k", "best passed for the KICK function = %f", best_path);
        kick_function(distance_matrix, path, nnodes, tour_length, k);
    }
    free(best_node_tour);
}

void kick_function(const double *distance_matrix, int *path, int nnodes, double *tour_length, int k)
{
    int count = 0;
    int is_odd = k % 2;
    if (is_odd == 1)
    {
        // chose a random number between 0 and nnodes - 5 at lease 4 nodes to swap
        int lower = 1;
        int upper = nnodes - 6;
        int start = (rand() % (upper - lower + 1)) + lower; // random number between 1 and nnodes - 5
        swap_array_piece(path, start, start + 1, start + 2, start + 3);
        DEBUG_COMMENT("heuristics.c:kick_function", "start=%d", start);
        *tour_length +=   distance_matrix[path[start - 1] * nnodes + path[start + 2]]  
                        + distance_matrix[path[start + 3] * nnodes + path[start]] 
                        + distance_matrix[path[start + 1] * nnodes + path[start + 4]] 
                        - distance_matrix[path[start - 1] * nnodes + path[start]]  
                        - distance_matrix[path[start + 1] * nnodes + path[start + 2]]  
                        - distance_matrix[path[start + 3] * nnodes + path[start + 4]];
        DEBUG_COMMENT("heuristics.c:kick_function", "distances to sum {%f, %f, %f}", distance_matrix[path[start - 1] * nnodes + path[start + 2]], distance_matrix[path[start + 3] * nnodes + path[start]], distance_matrix[path[start + 1] * nnodes + path[start + 4]]);
        DEBUG_COMMENT("heuristics.c:kick_function", "distances to subtract {%f, %f, %f}", distance_matrix[path[start - 1] * nnodes + path[start]], distance_matrix[path[start + 1] * nnodes + path[start + 2]], distance_matrix[path[start + 3] * nnodes + path[start + 4]]);
        DEBUG_COMMENT("heuristics.c:kick_function", "tour_length=%f", *tour_length);
        count += 3;

        while (count < k)
        {

            int lower = 0;
            int upper = nnodes - 6;
            int start = (rand() % (upper - lower + 1)) + lower;
            swap(path, start, start + 1);
            DEBUG_COMMENT("heuristics.c:kick_function", "start=%d", start);
            *tour_length += distance_matrix[path[start - 1] * nnodes + path[start + 1]] + distance_matrix[path[start + 2] * nnodes + path[start]] -
                            distance_matrix[path[start - 1] * nnodes + path[start]] - distance_matrix[path[start + 1] * nnodes + path[start + 2]];
            DEBUG_COMMENT("heuristics.c:kick_function", "tour_length=%f", *tour_length);
            count += 2;
        }
    }
    else
    {
        while (count < k)
        {
            int lower = 0;
            int upper = nnodes - 6;
            int start = (rand() % (upper - lower + 1)) + lower;
            swap(path, start, start + 1);
            DEBUG_COMMENT("heuristics.c:kick_function", "start=%d", start);
            *tour_length += distance_matrix[path[start - 1] * nnodes + path[start + 1]] + distance_matrix[path[start + 2] * nnodes + path[start]] -
                            distance_matrix[path[start - 1] * nnodes + path[start]] - distance_matrix[path[start + 1] * nnodes + path[start + 2]];
            DEBUG_COMMENT("heuristics.c:kick_function", "tour_length=%f", *tour_length);
            count += 2;
        }
    }
    CRITICAL_COMMENT("heuristics.c:kick_function", "tour_length=%f", *tour_length);
}
