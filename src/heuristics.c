#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "heuristics.h"
#include "greedy.h"
#include <string.h>
#include <math.h>

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

//-----------------------------------------------------------------------------------------------
// Tabu search

int tabuListContains(int n1, int n2, int tabuList[], int tabuListSize, int maxTabuSize)
{
    for (int i = 0; i < (tabuListSize < maxTabuSize ? tabuListSize : maxTabuSize); i++)
    {
        if (tabuList[i] == n1 || tabuList[i] == n2)
        {
            return 1; // Solution is in the tabu list
        }
    }
    return 0; // Solution is not in the tabu list
}

int tabu_stoppingCondition(int currentIteration, int nIterNotFoundImp, int nnodes)
{
    return currentIteration > nnodes * 10 || nIterNotFoundImp > nnodes / 10; // TODO setup stopping condition
}

int tabu_getTabuListSize(int currentIteration, int nnodes)
{
    // return (int)(currentIteration < 10e3 ? currentIteration / 10 : currentIteration / 100);
    return (int)(nnodes < 10e3 ? nnodes / 10 : nnodes / 100);
}

void tabu_search(const double *distance_matrix, int *path, int nnodes, double *tour_length, int maxTabuSize, time_t timelimit)
{
    time_t start_time = time(NULL);
    INFO_COMMENT("heuristic.c:tabu_search", "Starting tabu search");
    int tabuList[maxTabuSize]; // save only first node involved in operation
    int tabuListSize = 0;

    // 1. Set initial solution as best candidate
    int bestCandidate[nnodes];
    memcpy(bestCandidate, path, nnodes * sizeof(int));

    int currentIteration = 0;
    int nIterNotFoundImp = 0;
    int nIterNotFoundIncImp = 0;

    double min_delta;
    int operation[2];

    // 1.1. Find local optima -- intensification
    two_opt(distance_matrix, nnodes, path, tour_length, INFINITY);

    // // current path
    // *path
    // *tour_length

    // // current position tabu
    double cost_old_edge, cost_new_edge, cost_old_edge2, cost_new_edge2;
    // // endumbment
    int encumbment_path[nnodes];
    memcpy(encumbment_path, path, nnodes * sizeof(int));
    double encumbment_tour_length = *tour_length;

    // TODO clean tabulist after many number of iterations
    while (currentIteration > nnodes * 4 && nIterNotFoundImp > nnodes && difftime(start_time, time(NULL)) < timelimit)
    {
        currentIteration++;
        DEBUG_COMMENT("tabu_search", "iteration: %d", currentIteration);

        // FLAGS
        int incumb = 0;
        min_delta = INFTY;

        // 2. Generate neighboors
        // -> if update encumb : update right away
        // -> if decrease found : move and do not insert in tabu
        // -> if increase found : move and insert in tabu
        for (int i = 0; i < nnodes - 2; i++)
        {
            int k, k1;
            // for (int j = 0; j < nnodes / 3; j++) // TODO do the same in two opt -> reach last node and use j+1 % nnodes
            // {
            //     // make 2opt move
            //     k = randomBetween(0, nnodes);
            for (int j = 0; j < nnodes - 1; j++) // TODO do the same in two opt -> reach last node and use j+1 % nnodes
            {
                // make 2opt move
                k = j;
                k1 = (k + 1) % nnodes;
                cost_old_edge = distance_matrix[path[i] * nnodes + path[i + 1]];
                cost_new_edge = distance_matrix[path[i] * nnodes + path[k]];
                cost_old_edge2 = distance_matrix[path[k] * nnodes + path[k1]];
                cost_new_edge2 = distance_matrix[path[i + 1] * nnodes + path[k1]];
                double delta = (cost_new_edge + cost_new_edge2) - (cost_old_edge + cost_old_edge2);

                // if (delta < 0) // new edges are better
                if (*tour_length + delta < encumbment_tour_length)
                {
                    operation[0] = i;
                    operation[1] = k;
                    DEBUG_COMMENT("tabu_search", "improvement operation: %d %d tl= %lf", i, k, *tour_length);
                    incumb = 1;
                    min_delta = delta;
                    break;
                }
                else if (delta < min_delta && !tabuListContains(i, k, tabuList, tabuListSize, maxTabuSize))
                {
                    operation[0] = i;
                    operation[1] = k;
                    min_delta = delta;
                    DEBUG_COMMENT("tabu_search", "found tabu:\t node %d\t increase %lf", operation[0], min_delta);
                }
            }
            if (incumb)
                break;
        }

        // do selected move
        two_opt_move(path, operation[0], operation[1], nnodes);
        *tour_length = *tour_length + min_delta;

        if (incumb)
        { // updated incumbment
            DEBUG_COMMENT("tabu_search", "update incumb operation: %d %d tl= %lf", operation[0], operation[1], *tour_length);
            memcpy(encumbment_path, path, nnodes * sizeof(int));
            encumbment_tour_length = *tour_length;

            nIterNotFoundImp = 0;
            nIterNotFoundIncImp = 0;
        }
        else if (min_delta < 0)
        { // decrease current path
            DEBUG_COMMENT("tabu_search", "improve path: %d %d tl= %lf", operation[0], operation[1], *tour_length);
            nIterNotFoundImp = 0;
            nIterNotFoundIncImp++;
        }
        else
        {
            tabuList[tabuListSize % maxTabuSize] = operation[0];
            tabuListSize++;
            DEBUG_COMMENT("tabu_search", "new tabu move: node %d\t lenght %lf", operation[0], *tour_length);
            // #ifndef PRODUCTION
            //             for (int l = 0; l < (tabuListSize < maxTabuSize ? tabuListSize : maxTabuSize); l++)
            //             {
            //                 DEBUG_COMMENT("tabu_search", "tabu list: %d", tabuList[l]);
            //             }
            // #endif
        }

        INFO_COMMENT("tabu_search", "end iteration: %d\t lenght %lf\t encumbment %lf", currentIteration, *tour_length, encumbment_tour_length);
    }

    // CLOSE UP -  return best encumbment
    path = encumbment_path;
    *tour_length = encumbment_tour_length;
    INFO_COMMENT("tabu_search", "end tabu search: %lf", *tour_length);
}
