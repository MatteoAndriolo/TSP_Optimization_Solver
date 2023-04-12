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
    return currentIteration > 200;
}

void tabu_search(const double *distance_matrix, int *path, int nnodes, double *tour_length, int maxTabuSize)
{
    // TODO in order to get neighbour iterate all 2opt moves, calculate tourlenght
    //    -  if tourlenght < best tourlenght, then update patheven if tabu
    //    -  pick the neighbout wit best fitness

    // Process
    // Start with solution in path
    // 2opt heuristics until reach a local minima
    // when reached local minima search for the move with minimum increase of tour lenght  (doing a 2opt)
    //  then add one of the nodes in the tabu list
    //  ! move must be done in nodes not in the tabu list
    //  ! ! it can be accepted only if improves the incumbment (best solution found)
    maxTabuSize = (int)(nnodes < 10e3 ? nnodes / 10 : nnodes / 100);
    int tabuList[maxTabuSize]; // save only first node involved in operation
    int tabuListSize = 0;

    int bestCandidate[nnodes];
    memcpy(bestCandidate, path, nnodes * sizeof(int)); // Set initial solution as best candidate

    int currentIteration = 0;
    // 2OPT --------------------------------------------------------------
    int foundImprovement = 0;
    double min_increase = INFTY;
    int improvementOperation[2];
    double pendingTourLenght;
    int tabuNodes[2];
    double cost_old_edge, cost_new_edge, cost_old_edge2, cost_new_edge2;
    int encumbment_path[nnodes];
    memcpy(encumbment_path, path, nnodes * sizeof(int));
    double encumbment_tour_length = *tour_length;
    // todo clean tabulist after many number of iterations
    while (!stoppingCondition(currentIteration))
    {
        currentIteration++;
        DEBUG_COMMENT("tabu_search", "iteration: %d", currentIteration);
        min_increase = INFTY;
        foundImprovement = 0;
        pendingTourLenght = *tour_length;
        for (int i = 0; i < nnodes - 2; i++)
        {
            for (int j = i + 1; j < nnodes - 1; j++)
            {
                cost_old_edge = distance_matrix[path[i] * nnodes + path[i + 1]];
                cost_new_edge = distance_matrix[path[i] * nnodes + path[j]];
                cost_old_edge2 = distance_matrix[path[j] * nnodes + path[j + 1]];
                cost_new_edge2 = distance_matrix[path[i + 1] * nnodes + path[j + 1]];
                double delta = (cost_new_edge + cost_new_edge2) - (cost_old_edge + cost_old_edge2);

                // if two opt improve path
                if (*tour_length + delta < *tour_length)
                {
                    // if better than encumbment or improve tour lenght and not in tabu list
                    if (*tour_length + delta < encumbment_tour_length || (*tour_length + delta < pendingTourLenght && !tabuListContains(i, tabuList, tabuListSize, maxTabuSize) && !tabuListContains(j, tabuList, tabuListSize, maxTabuSize)))
                    {
                        foundImprovement = 1;
                        improvementOperation[0] = i;
                        improvementOperation[1] = j;
                        pendingTourLenght = *tour_length + delta;
                        // two_opt_move(path, i, j, nnodes);
                        //*tour_length += delta;
                        DEBUG_COMMENT("tabu_search", "found improvement: %lf", pendingTourLenght);
                    }
                }
                // else if (delta < min_increase && delta>0 && !tabuListContains(i, tabuList, tabuListSize, maxTabuSize) && !tabuListContains(j, tabuList, tabuListSize, maxTabuSize))
                else if (delta < min_increase && !tabuListContains(i, tabuList, tabuListSize, maxTabuSize) && !tabuListContains(j, tabuList, tabuListSize, maxTabuSize))
                {
                    min_increase = delta;
                    tabuNodes[0] = i;
                    tabuNodes[1] = j;
                    DEBUG_COMMENT("tabu_search", "found tabu:\t node %d\t increase %lf", tabuNodes[0], min_increase);
                }
            }
        }
//        // case first node and last node
//        DEBUG_COMMENT("tabu_search", "lokinh into special");
//        int i = nnodes - 1;
//        for (int j = 0; j <= nnodes - 2; j++)
//        {
//            cost_old_edge = distance_matrix[path[i] * nnodes + path[0]];
//            cost_new_edge = distance_matrix[path[i] * nnodes + path[j]];
//            cost_old_edge2 = distance_matrix[path[j] * nnodes + path[j + 1]];
//            cost_new_edge2 = distance_matrix[path[0] * nnodes + path[j + 1]];
//            double delta = (cost_new_edge + cost_new_edge2) - (cost_old_edge + cost_old_edge2);
//
//            // if two opt improve path
//            if (*tour_length + delta < *tour_length)
//            {
//                // if better than encumbment or improve tour lenght and not in tabu list
//                if (*tour_length + delta < encumbment_tour_length || (*tour_length + delta < pendingTourLenght && !tabuListContains(i, tabuList, tabuListSize, maxTabuSize) && !tabuListContains(j, tabuList, tabuListSize, maxTabuSize)))
//                {
//                    foundImprovement = 1;
//                    improvementOperation[0] = i;
//                    improvementOperation[1] = j;
//                    pendingTourLenght = *tour_length + delta;
//                    // two_opt_move(path, i, j, nnodes);
//                    //*tour_length += delta;
//                    DEBUG_COMMENT("tabu_search", "found encumbment improvement : %lf", pendingTourLenght);
//                }
//            }
//            // else if (delta < min_increase && delta>0 && !tabuListContains(i, tabuList, tabuListSize, maxTabuSize) && !tabuListContains(j, tabuList, tabuListSize, maxTabuSize))
//            else if (delta < min_increase && !tabuListContains(i, tabuList, tabuListSize, maxTabuSize) && !tabuListContains(j, tabuList, tabuListSize, maxTabuSize))
//            {
//                min_increase = delta;
//                tabuNodes[0] = i;
//                tabuNodes[1] = j;
//                DEBUG_COMMENT("tabu_search", "found tabu:\t node %d\t increase %lf", tabuNodes[0], min_increase);
//            }
//        }
//        // end special case

        DEBUG_COMMENT("tabu_search", foundImprovement ? "found improvement" : "no improvement");
        if (foundImprovement)
        {
            two_opt_move(path, improvementOperation[0], improvementOperation[1], nnodes);
            *tour_length = pendingTourLenght;
            DEBUG_COMMENT("tabu_search", "improvement operation: %d %d tl= %lf", improvementOperation[0], improvementOperation[1], *tour_length);
        }
        else
        {
            tabuList[tabuListSize % maxTabuSize] = tabuNodes[0];
            tabuListSize++;
            two_opt_move(path, tabuNodes[0], tabuNodes[1], nnodes);
            *tour_length += min_increase;
            DEBUG_COMMENT("tabu_search", "new tabu move: node %d\t lenght %lf", tabuNodes[0], *tour_length);

            for (int l = 0; l < maxTabuSize; l++)
            {
                DEBUG_COMMENT("tabu_search", "tabu list: %d", tabuList[l]);
            }
        }

        if (*tour_length <= encumbment_tour_length)
        {
            memcpy(encumbment_path, path, nnodes * sizeof(int));
            encumbment_tour_length = *tour_length;
            DEBUG_COMMENT("tabu_search", "new encumbment: %lf", encumbment_tour_length);
        }
        INFO_COMMENT("tabu_search", "end iteration: %d\t lenght %lf\t encumbment %lf", currentIteration, *tour_length, encumbment_tour_length);
    }

    path = encumbment_path;
    *tour_length = encumbment_tour_length;
}