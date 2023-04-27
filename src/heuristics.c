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
        // printf("Time left: %.0f seconds\n", difftime(end_time, time(NULL)));
        two_opt(distance_matrix, nnodes, path, tour_length);
        kick_function(distance_matrix, path, nnodes, tour_length, k);
    }
}

int randomBetween(int lowerBound, int upperBound)
{
    int randomBetween = (rand() % (upperBound - lowerBound + 1)) + lowerBound;
    return randomBetween;
}

void kick_function(const double *distance_matrix, int *path, int nnodes, double *tour_length, int k)
{
    int found = 0;
    int index_0, index_1, index_2, index_3, index_4;
    while (found != 4)
    {
        found = 0;
        index_0 = randomBetween(1, nnodes);
        found++;
        index_1 = randomBetween(index_0 + 1, nnodes);
        if (nnodes - index_1 > 6)
        {
            CRITICAL_COMMENT("ENTER1", "enter1");
            index_2 = randomBetween(index_1 + 1, nnodes);
            found ++;
        }
        if (nnodes - index_2 > 4)
        {
            CRITICAL_COMMENT("ENTER2", "enter2");
            index_3 = randomBetween(index_2 + 1, nnodes);
            found ++;
        }
        if (nnodes - index_3 > 2)
        {
            CRITICAL_COMMENT("ENTER3", "enter3");
            index_4 = randomBetween(index_3 + 1, nnodes);
            found ++; 
        }
        if (found == 4){
            index_0 --;
            index_1 --; 
            index_2 --; 
            index_3 --;
            index_4 --;
            DEBUG_COMMENT("heuristics.c:kick_function", "found 5 indexes{%d, %d, %d, %d, %d}", index_0, index_1, index_2, index_3, index_4);
        }
    }
    //---------------------------------------------------------------------------------------


}