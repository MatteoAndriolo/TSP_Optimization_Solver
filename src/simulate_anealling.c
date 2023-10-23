#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "simulate_anealling.h"

void split_path(const double *distance_matrix, int *path, int nnodes, double *tour_length)
{
    INFO_COMMENT("simulate_anealling.c:split_path", "Splitting path");
    srand(time(NULL));

    int randomValue_1, randomValue_2;
    do
    {
        randomValue_1 = rand() % (nnodes - 1) + 1;
        randomValue_2 = rand() % (nnodes - 1) + 1;

    } while (randomValue_2 == randomValue_1);

    int min = (randomValue_1 < randomValue_2) ? randomValue_1 : randomValue_2;
    int max = (randomValue_1 > randomValue_2) ? randomValue_1 : randomValue_2;

    DEBUG_COMMENT("simulate_anealling.c:split_path", "randomValue_1=%d, randomValue_2=%d", randomValue_1, randomValue_2);

    *tour_length = *tour_length - distance_matrix[path[min - 1] * nnodes + path[min]] - distance_matrix[path[max - 1] * nnodes + path[max]];

    DEBUG_COMMENT("simulate_anealling.c:split_path only subtraction", "tour_length=%f", *tour_length);
    // Allocate memory for the new path
    int *new_path = (int *)malloc(nnodes * sizeof(int));

    // Copy the first portion before the split point
    memcpy(new_path, path, min * sizeof(int));

    // Copy the second portion after the split point
    memcpy(new_path + min, path + max, (nnodes - max) * sizeof(int));

    // Copy the third portion between the two split points
    memcpy(new_path + min + nnodes - max, path + min, (max - min) * sizeof(int));

    // Update the path and tour_length
    memcpy(path, new_path, nnodes * sizeof(int));

    free(new_path);
    // Clean up the allocated memory

    *tour_length = *tour_length + distance_matrix[path[min - 1] * nnodes + path[min]] + distance_matrix[path[max - 1] * nnodes + path[max]];
    DEBUG_COMMENT("simulate_anealling.c:split_path", "tour_length=%f", *tour_length);
    char *tmp;
    tmp = getPath(path, nnodes);
    DEBUG_COMMENT("tspcplex.c:TSPopt", "path = %s", tmp);
    free(tmp);
}
void simulate_anealling(const double *distance_matrix, int *path, int nnodes, double *tour_length, int k, int duration)
{
    INFO_COMMENT("simulate_anealling.c:simulate_anealling", "Simulate anealling algorithm");
    double x_best = *tour_length;
    double x_new = *tour_length;
    int T_min = 10;
    int K_max = 100000;
    int T_i = 100;
    double diff;
    double p;
    DEBUG_COMMENT("simulate_anealling.c:simulate_anealling", "x_best = %f ", x_best);
    DEBUG_COMMENT("simulate_anealling.c:simulate_anealling", "tour_length = %f ", *tour_length);

    while (T_i > T_min && K_max > 0)
    {
        int *new_path = (int *)malloc(nnodes * sizeof(int));

        split_path(distance_matrix, new_path, nnodes, &x_new);

        diff = x_new - x_best;

        if (diff < 0)
        {
            x_best = x_new;
            DEBUG_COMMENT("simulate_anealling.c:simulate_anealling, diff is < 0", "x_best = %f ", x_best);
        }

        if (diff >= 0)
        {
            DEBUG_COMMENT("simulate_anealling.c:simulate_anealling, diff is >= 0", "x_best = %f ", x_best);
            p = exp(-diff / T_i); // Note the negative sign here
            double randomValue = (double)rand() / RAND_MAX;

            if (randomValue >= p)
            {
                x_best = x_new;
            }
            else
            {
                x_best = x_best;
            }
        }
        K_max--;
        T_i = (int)(p * T_i);
        *path = x_best;
        free(new_path);
    }
}