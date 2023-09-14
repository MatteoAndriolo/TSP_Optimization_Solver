#include "heuristics.h"

void vnp_k(const double *distance_matrix, int *path, int nnodes, double *tour_length, int k, int duration)
{
    time_t start_time = time(NULL);
    time_t end_time = start_time + duration;
    // while non finisce il tempo
    double best_tour = INFINITY;
    int *best_path = malloc(nnodes * sizeof(int));
    INFO_COMMENT("heuristics.c:vnp_k", "starting the huristics loop for vnp_k");
    while (difftime(end_time, time(NULL)) > 0)
    {
        printf("Time left: %.0f seconds\n", difftime(end_time, time(NULL)));
        two_opt(distance_matrix, nnodes, path, tour_length, INFINITY);
        kick_function(distance_matrix, path, nnodes, tour_length, k);
        if (best_tour > *tour_length)
        {
            best_tour = *tour_length;
            memcpy(best_path, path, nnodes * sizeof(int));
            CRITICAL_COMMENT("heuristics.c:vnp_k", "found a better tour with length %lf", best_tour);
            log_path(best_path, nnodes);
        }
    }
    *tour_length = best_tour;
    path = best_path;
    INFO_COMMENT("heuristics.c:vnp_k", "finished the huristics loop for vnp_k, best path found= %lf", best_tour);
    log_path(path, nnodes);
}

void kick_function(const double *distance_matrix, int *path, int nnodes, double *tour_length, int k)
{
    int found = 0;
    int index_0, index_1, index_2, index_3, index_4;
    while (found != 4)
    {
        found = 0;
        index_0 = 1;
        found++;
        index_1 = randomBetween(index_0 + 1, nnodes);
        if (nnodes - index_1 > 6)
        {
            index_2 = randomBetween(index_1 + 2, nnodes);
            found++;
        }
        if (nnodes - index_2 > 4)
        {
            index_3 = randomBetween(index_2 + 2, nnodes);
            found++;
        }
        if (nnodes - index_3 > 2)
        {
            index_4 = randomBetween(index_3 + 2, nnodes);
            found++;
        }
        if (found == 4)
        {
            index_0--;
            index_1--;
            index_2--;
            index_3--;
            index_4--;
            // DEBUG_COMMENT("heuristics.c:kick_function", "found 5 indexes{%d, %d, %d, %d, %d}", index_0, index_1, index_2, index_3, index_4);
        }
    }
    //---------------------------------------------------------------------------------------
    int *final_path = malloc(nnodes * sizeof(int));
    int count = 0;
    for (int i = 0; i < index_1; i++)
        final_path[count++] = path[i];
    for (int i = index_4; i < nnodes; i++)
        final_path[count++] = path[i];
    for (int i = index_4 - 1; i >= index_3; i--)
        final_path[count++] = path[i];
    for (int i = index_2; i < index_3; i++)
        final_path[count++] = path[i];
    for (int i = index_2 - 1; i >= index_1; i--)
        final_path[count++] = path[i];
    //---------------------------------------------------------------------------------------
    for (int i = 0; i < nnodes; i++)
        path[i] = final_path[i];
    *tour_length = 0;
    for (int i = 1; i < nnodes; i++)
    {
        *tour_length += distance_matrix[path[i - 1] * nnodes + path[i]];
    }
    *tour_length += distance_matrix[path[0] * nnodes + path[nnodes - 1]];
}
