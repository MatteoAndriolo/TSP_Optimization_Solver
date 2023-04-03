#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "heuristics.h"
#include "greedy.h"

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
    start[0] = rand() % (nnodes / k);
    for (int i = 0; i < k; i++)
        start[i] = start[i - 1] + 5 + rand() % ((nnodes - start[i - 1]) / k - 1);

    if (k % 2 == 1)
    {
        // if odd
        // ABC -> BAC == (i j-1) (j k-1) (k i-1)
        // ABC -> BAC == (j k-1) (i j-1) (k i-1) to add (k-1 i) (j-1 k) to remove (j-1 j) (k-1 k) (i-1 i)
        for (int i = 0; i < k - 1; i = i + 2)
        {
            int I = start[i];
            int J = start[i + 1];
            int K = start[i + 2];
            swap_array_piece(path, I, J - 1, J, K - 1);
            DEBUG_COMMENT("heuristics:kick_function", "swap_array_piece(%d, %d, %d, %d)", start[i], start[i + 1] - 1, start[i + 1], start[i + 2] - 1);
            /**tour_length += distance_matrix[path[K-1] * nnodes + path[I]]
                          + distance_matrix[path[J-1] * nnodes + path[K]]
                          + distance_matrix[path[I-1] * nnodes + path[J]]
                          - distance_matrix[path[J-1] * nnodes + path[J]]
                          - distance_matrix[path[K-1] * nnodes + path[K]]
                          - distance_matrix[path[I-1] * nnodes + path[I]];*/
        }
    }
    else
    {
        // if even
        // ABCD -> CABD == (i j-1) (j k-1) (k l-1) (l i-1)
        for (int i = 0; i < k - 1; i = i + 2)
        {
            swap_array_piece(path, start[i], start[i + 1] - 1, start[i + 2], start[i + 3] - 1);
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

int tabuListContains(int n1, int tabuList[], int tabuListSize, int maxTabuSize)
{
    for (int i = 0; i < (tabuListSize < maxTabuSize ? tabuListSize : maxTabuSize); i++)
    {
        if (tabuList[i] == n1)
        {
            return 1; // Solution is in the tabu list
        }
    }
    return 0; // Solution is not in the tabu list
}

int stoppingCondition(int currentIteration)
{
    return currentIteration > 2;
}

void tabu_search(const double *distance_matrix, int *path, int nnodes, double *tour_length, int maxTabuSize)
{
    // TODO in order to get neighbour iterate all 2opt moves, calculate tourlenght
    //    -  if tourlenght < best tourlenght, then update patheven if tabu
    //    -  pick the neighbout wit best fitness
    int tabuList[maxTabuSize]; // save only first node involved in operation
    int tabuListSize = 0;

    int bestCandidate[nnodes];
    memcpy(bestCandidate, path, nnodes * sizeof(int)); // Set initial solution as best candidate
    double candidateTourLenght;

    int neighborhoodSize = nnodes / 10;

    int currentIteration = 0;
    while (!stoppingCondition(currentIteration))
    {
        // in order to get neighbour of distance i have to change only one
        int i = 0;
        while (i < neighborhoodSize)
        {
            int n1, n2;
            n1 = rand() % nnodes;
            n2 = rand() % nnodes;
            if (n1 == n2)
            {
                continue;
            }

            double new_candidate_tour_lenght = two_opt_move(distance_matrix, bestCandidate, nnodes, *tour_length, n1, n2, 0);

            if (new_candidate_tour_lenght < *tour_length)
            {
                candidateTourLenght = two_opt_move(distance_matrix, bestCandidate, nnodes, *tour_length, n1, n2, 1); // save move
                two_opt(distance_matrix, nnodes, bestCandidate, &candidateTourLenght);
            }
            else if (!tabuListContains(n1, tabuList, tabuListSize, maxTabuSize) && new_candidate_tour_lenght < candidateTourLenght)
            {
                candidateTourLenght = new_candidate_tour_lenght;
                two_opt_move(distance_matrix, bestCandidate, nnodes, *tour_length, n1, n2, 1); // save move
                tabuList[tabuListSize % maxTabuSize] = n1;
                tabuListSize++;
            }
            i++;
        }

        if (candidateTourLenght < *tour_length)
        {
            *tour_length = candidateTourLenght;
            memcpy(path, bestCandidate, nnodes * sizeof(int));
        }
        tabuListSize=0;
    }
}
