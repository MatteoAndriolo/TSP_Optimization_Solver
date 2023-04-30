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
    double best_tour = INFINITY;
    int* best_path = malloc (nnodes * sizeof(int));
    INFO_COMMENT("heuristics.c:vnp_k", "starting the huristics loop for vnp_k");
    while (difftime(end_time, time(NULL)) > 0)
    {
        printf("Time left: %.0f seconds\n", difftime(end_time, time(NULL)));
        two_opt(distance_matrix, nnodes, path, tour_length, INFINITY);
        kick_function(distance_matrix, path, nnodes, tour_length, k);
        if (best_tour > *tour_length){
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
            found ++;
        }
        if (nnodes - index_2 > 4)
        {
            index_3 = randomBetween(index_2 + 2, nnodes);
            found ++;
        }
        if (nnodes - index_3 > 2)
        {
            index_4 = randomBetween(index_3 + 2, nnodes);
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
    int* final_path = malloc(nnodes * sizeof(int));
    int count = 0;
    for (int i = 0; i < index_1; i++) final_path[count++] = path[i];
    for (int i = index_4; i < nnodes; i++) final_path[count++] = path[i];
    for (int i = index_4 - 1; i >= index_3; i--) final_path[count++] = path[i];
    for (int i = index_2; i < index_3; i ++) final_path[count++] = path[i]; 
    for (int i = index_2 - 1; i >= index_1; i--) final_path[count++] = path[i];
    //---------------------------------------------------------------------------------------
    for (int i = 0; i < nnodes; i++) path[i] = final_path[i];
    *tour_length = 0; 
    for (int i = 1; i < nnodes; i++){
        *tour_length += distance_matrix[path[i-1] * nnodes + path[i]];
    }
    *tour_length += distance_matrix[path[0] * nnodes + path[nnodes - 1]];
}

//-----------------------------------------------------------------------------------------------
// Tabu search

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
    return currentIteration > 200; //TODO improve stopping condition
}

void tabu_search(const double *distance_matrix, int *path, int nnodes, double *tour_length, int maxTabuSize)
{
    maxTabuSize = (int)(nnodes < 10e3 ? nnodes / 10 : nnodes / 100);
    int tabuList[maxTabuSize]; // save only first node involved in operation
    int tabuListSize = 0;

    int bestCandidate[nnodes];
    memcpy(bestCandidate, path, nnodes * sizeof(int)); // Set initial solution as best candidate

    int currentIteration = 0;
    int foundImprovement = 0;
    double min_increase = INFTY;
    int improvementOperation[2];
    double pendingTourLenght;
    int tabuNodes[2];
    double cost_old_edge, cost_new_edge, cost_old_edge2, cost_new_edge2;
    int encumbment_path[nnodes];
    memcpy(encumbment_path, path, nnodes * sizeof(int));
    double encumbment_tour_length = *tour_length;
    //TODO clean tabulist after many number of iterations
    while (!stoppingCondition(currentIteration))
    {
        currentIteration++;
        DEBUG_COMMENT("tabu_search", "iteration: %d", currentIteration);
        min_increase = INFTY;
        foundImprovement = 0;
        pendingTourLenght = *tour_length;
        for (int i = 0; i < nnodes - 2; i++)
        {
            for (int j = i + 1; j < nnodes; j++) // TODO do the same in two opt -> reach last node and use j+1 % nnodes
            {
                int j1 = (j + 1) % nnodes;
                cost_old_edge = distance_matrix[path[i] * nnodes + path[i + 1]];
                cost_new_edge = distance_matrix[path[i] * nnodes + path[j]];
                cost_old_edge2 = distance_matrix[path[j] * nnodes + path[j1]];
                cost_new_edge2 = distance_matrix[path[i + 1] * nnodes + path[j1]];
                double delta = (cost_new_edge + cost_new_edge2) - (cost_old_edge + cost_old_edge2);

                if (*tour_length + delta < *tour_length)
                {
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

            for (int l = 0; l < (tabuListSize < maxTabuSize? tabuListSize : maxTabuSize); l++)
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
        //log_path(path, nnodes);
    }
    path = encumbment_path;
    *tour_length = encumbment_tour_length;
}