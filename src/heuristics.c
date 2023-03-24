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
    //while (difftime(end_time, time(NULL)) > 0)
    while(1)
    {
        CRITICAL_COMMENT("heuristics:vnp_k", "Time left: %.0f seconds", difftime(end_time, time(NULL)));
        printf("Time left: %.0f seconds\n", difftime(end_time, time(NULL)));
        // chiamre two_opt
        two_opt(distance_matrix, nnodes, path, tour_length);
        INFO_COMMENT("heuristics:vnp_k", "two_opt done");
        kick_function(distance_matrix, path, nnodes, tour_length, k);
        INFO_COMMENT("heuristics:vnp_k", "kick_function done");
    }
}

void kick_function(const double *distance_matrix, int *path, int nnodes, double *tour_length, int k)
{
    srand(time(NULL));
    // create array of k random numbers
    int *start = (int *)malloc(k * 2 * sizeof(int));
    for (int i = 0; i < k; i++)
        start[i] = rand() % nnodes;
    INFO_COMMENT("heuristics:kick_function", "start array created");
    if(k % 2 == 1)
    {
        for (int i = 0; i < k - 1; i = i + 2)
        {
            swap_array_piece(path, start[i], start[i + 1] - 1, start[i + 1], start[i + 2] - 1); // start at index i, finisc at next index - 1, start at next index, finish at next next index -1
        }
    }
    else
    {
        // if even
        for (int i = 0; i < k - 1; i = i + 2)
        {
            swap_array_piece(path, start[i], start[i + 1] - 1, start[i + 2], start[i + 3] - 1);
        }
    }
}