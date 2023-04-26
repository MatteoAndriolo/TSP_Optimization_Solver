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

// Define the function to generate 3 sequential nodes with random values
void generateNodes(int* firstValue, int* secondValue, int* thirdValue) {
    // Initialize the random number generator with the current time
    srand(time(NULL));

    // Generate random values for the nodes
    *firstValue = rand() % 100; // Generate a random value between 0 and 99
    *secondValue = rand() % (100 - *firstValue) + *firstValue + 1; // Generate a random value between *firstValue + 1 and 99
    *thirdValue = rand() % (100 - *secondValue) + *secondValue + 1; // Generate a random value between *secondValue + 1 and 99
}

void kick_function(const double *distance_matrix, int *path, int nnodes, double *tour_length, int k)
{


    for (int i = 0; i < k;i++){
        int firstValue, secondValue, thirdValue;
        generateNodes(&firstValue, &secondValue, &thirdValue);
        DEBUG_COMMENT("heuristics.c:kick_function", " [%d,%d,%d]", firstValue, secondValue, thirdValue);

    }
    CRITICAL_COMMENT("heuristics.c:kick_function", "tour_length=%f", *tour_length);
}
