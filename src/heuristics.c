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
    INFO_COMMENT("heuristics.c:vnp_k", "starting the huristics loop for vnp_k");
    while (difftime(end_time, time(NULL)) > 0)
    {
        printf("Time left: %.0f seconds\n", difftime(end_time, time(NULL)));
        two_opt(distance_matrix, nnodes, path, tour_length, INFINITY);
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
    return currentIteration > 200; // TODO improve stopping condition
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
    // TODO clean tabulist after many number of iterations
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

            for (int l = 0; l < (tabuListSize < maxTabuSize ? tabuListSize : maxTabuSize); l++)
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
        // log_path(path, nnodes);
    }
    path = encumbment_path;
    *tour_length = encumbment_tour_length;
}

//---------------------------------------------------------------------------------------------
// Genetic Algorithm

double getFitness(const double *distance_matrix, int *path, int nnodes)
{
    if (!feasiblePath(path, nnodes))
        ERROR_COMMENT("heuristic.c:getFitness", "path not feasible");
    double fitness = get_tour_length(path, nnodes, distance_matrix);
    return fitness;
}

int getChampion(Individual *champion, Individual **population, int populationSize, int nnodes)
{
    double min = population[0]->fitness;
    int ind_min = 0;
    for (int i = 1; i < populationSize; i++)
    {
        if (population[i]->fitness < min)
        {
            min = population[i]->fitness;
            ind_min = i;
        }
    }
    champion->fitness = min;
    champion->path = population[ind_min]->path;
    return ind_min;
}

int *genetic_merge_parents(Individual *child, const Individual *p1, const Individual *p2, const double *distance_matrix, const int nnodes)
{
    int *isMissing = malloc(nnodes * sizeof(int));
    for (int j = 0; j < nnodes; j++)
    {
        child->path[j] = -1;
        isMissing[j] = 1;
    }
    // which portion of port 1 i will use?
    int portion1 = (nnodes / 4) + rand() % (nnodes / 2); // portion between quarters
    int countNodes;
    for (countNodes = 0; countNodes < portion1; countNodes++)
    {
        child->path[countNodes] = p1->path[countNodes];
        isMissing[p1->path[countNodes]] = 0;
    }

    for (int j = countNodes; j < nnodes; j++)
    {
        int toAdd = p2->path[j]; // FIXME start adding from the second portion of the path
        int found = 0;
        for (int k = 0; k < portion1; k++)
        {
            if (toAdd == child->path[k])
            {
                found = 1;
                break;
            }
        }
        if (!found)
        {
            child->path[countNodes++] = toAdd;
            isMissing[toAdd] = 0;
        }
    }

    for (int j = 0; j < nnodes; j++)
    {
        if (isMissing[j])
        {
            child->path[countNodes++] = j;
        }
    }

    child->fitness = getFitness(distance_matrix, child->path, nnodes);
    return isMissing;
}

void genetic_repair_child(Individual *child, const double *distance_matrix, int nnodes, int effort)
{
    two_opt(distance_matrix, nnodes, child->path, &child->fitness, effort);
    child->fitness = getFitness(distance_matrix, child->path, nnodes);
}

double normalizeFitness(double fitness, double maxFitness, double minFitness)
{
    return (fitness - minFitness) / (maxFitness - minFitness) * 100;
}

void genetic_algorithm(const double *distance_matrix, int *path, int nnodes, double *tour_length, int populationSize, int iterations)
{
    INFO_COMMENT("heuristic::genetic_algorithm", "Starting genetic algorithm with population size %d and %d iterations", populationSize, iterations);

    // Generate random population
    Individual *population = malloc(populationSize * sizeof(Individual));
    for (int i = 0; i < populationSize; i++)
    {
        // population[i].path = malloc(nnodes * sizeof(int));
        population[i].path = malloc(nnodes * sizeof(int));
        generate_random_path(population[i].path, nnodes);
        population[i].fitness = getFitness(distance_matrix, population[i].path, nnodes);
        DEBUG_COMMENT("heuristic::genetic_algorithm", "Individual %d", i);
        DEBUG_COMMENT("heuristic::genetic_algorithm", "path: %s", getPath(population[i].path, nnodes));
        DEBUG_COMMENT("heuristic::genetic_algorithm", "fitness: %lf", population[i].fitness);
    }

    DEBUG_COMMENT("heuristic::genetic_algorithm", "Generated random population");

    // indexChampion = getChampion(&champion, &population, populationSize, nnodes);
    /**********************/
    double min = population[0].fitness;
    double max = 0;
    int ind_min = 0;
    for (int i = 1; i < populationSize; i++)
    {
        if (population[i].fitness < min)
        {
            min = population[i].fitness;
            ind_min = i;
        }
        if (population[i].fitness > max)
        {
            max = population[i].fitness;
        }
    }
    DEBUG_COMMENT("heuristic::getChampion", "Champion: %d", ind_min);
    int indexChampion = ind_min;
    Individual champion;
    champion.fitness = min;
    champion.path = population[ind_min].path;

    /** ENCUMBENT **/
    Individual encumbment = {NULL, INFINITY};
    double encumbment_fitness = INFINITY;
    int *encumbment_path = malloc(nnodes * sizeof(int));
    if (champion.fitness < encumbment_fitness)
    {
        encumbment_fitness = champion.fitness;
        memcpy(encumbment_path, champion.path, nnodes * sizeof(int));
    }

    /***********************************/
    /* Start generating new population */
    /***********************************/
    for (int iter = 0; iter < iterations; iter++)
    {
        printf("Iteration %d\n", iter);
        INFO_COMMENT("heuristic::genetic_algorithm", "Iteration %d", iter);

        int pps = populationSize * 1.3;
        Individual *newGeneration = malloc(pps * sizeof(Individual));
        double bestFitnessGeneration = champion.fitness;
        double maxFitnessGeneration = 0;
        double minFitnessGeneration = INFINITY;

        int probChoseChampion = 70;
        for (int i = 0; i < populationSize; i++)
        {
            // select parents
            int indexParent1 = rand() % 100 > probChoseChampion ? rand() % populationSize : indexChampion;
            int indexParent2 = rand() % populationSize;
            while (indexParent1 == indexParent2)
            {
                indexParent2 = rand() % populationSize;
            }
            DEBUG_COMMENT("heuristic::genetic_algorithm", "Selected parents %d and %d", indexParent1, indexParent2);

            /**
             * merge parents
             */
            Individual child;
            child.path = malloc(nnodes * sizeof(int));
            int *isMissing = malloc(nnodes * sizeof(int));
            for (int j = 0; j < nnodes; j++)
            {
                child.path[j] = -1;
                isMissing[j] = 1;
            }

            int portion1 = (nnodes / 4) + rand() % (nnodes / 2); // portion between quarters
            int countNodes;
            for (countNodes = 0; countNodes < portion1; countNodes++)
            {
                child.path[countNodes] = population[indexParent1].path[countNodes];
                isMissing[population[indexParent1].path[countNodes]] = 0;
            }

            for (int j = countNodes; j < nnodes; j++)
            {
                int toAdd = population[indexParent2].path[j]; // FIXME start adding from the second portion of the path
                int found = 0;
                for (int k = 0; k < portion1; k++)
                {
                    if (toAdd == child.path[k])
                    {
                        found = 1;
                        break;
                    }
                }
                if (!found)
                {
                    child.path[countNodes++] = toAdd;
                    isMissing[toAdd] = 0;
                }
            }

            DEBUG_COMMENT("heuristic::genetic_algorithm", "Merged before completing %s", getPath(child.path, nnodes));

            int count = 0;
            for (int j = 0; j < nnodes; j++)
            {
                if (isMissing[j])
                {
                    count++;
                    child.path[countNodes++] = j;
                }
            }
            DEBUG_COMMENT("heuristic::genetic_algorithm", "Merged after completing %s", getPath(child.path, nnodes));
            child.fitness = getFitness(distance_matrix, child.path, nnodes);

            /**********/
            /* Repair */
            /**********/
            genetic_repair_child(&child, distance_matrix, nnodes, count);

            newGeneration[i] = child;

            if (newGeneration[i].fitness < bestFitnessGeneration)
            {
                bestFitnessGeneration = newGeneration[i].fitness;
                indexChampion = i;
            }
            if (newGeneration[i].fitness > maxFitnessGeneration)
            {
                maxFitnessGeneration = newGeneration[i].fitness;
            }
            if (newGeneration[i].fitness < minFitnessGeneration)
            {
                minFitnessGeneration = newGeneration[i].fitness;
            }
        }
        INFO_COMMENT("heuristic::genetic_algorithm", "Champion of iteration %d has fitness %lf", iter, bestFitnessGeneration);

        /*************/
        /* Selection */
        /*************/
        double sumFitnessGeneration = 0;
        DEBUG_COMMENT(" awdawd ", "min %lf", minFitnessGeneration);
        for (int k = 0; k < pps; k++)
        {
            DEBUG_COMMENT("heuristic.c", "%lf, %lf", newGeneration[k].fitness * 10 / minFitnessGeneration, exp(newGeneration[k].fitness * 10 / minFitnessGeneration));
            sumFitnessGeneration += exp(newGeneration[k].fitness * 10 / minFitnessGeneration);
        }
        printf("denominator %lf\n", sumFitnessGeneration);

        while (pps > populationSize)
        {
            int ind = rand() % pps;
            double prob = exp(normalizeFitness(newGeneration[ind].fitness, maxFitnessGeneration, minFitnessGeneration));
            double p2 = rand() % 100;

            if (p2 > prob)
            {
                memcpy(newGeneration + pps - 1, newGeneration + ind, sizeof(Individual));
                pps--;
                DEBUG_COMMENT("heuristic::genetic_algorithm", "Individual %d removed, remaining: %d", ind, pps);
            }
            else
            {
                DEBUG_COMMENT("heuristic::genetic_algorithm", "p2=%lf, prob= %lf ", p2, prob);
            }
        }

        champion.fitness = bestFitnessGeneration;
        champion.path = newGeneration[indexChampion].path;
        for (int i = 0; i < populationSize; i++)
        {
            free(population[i].path);
        }
        free(population);
        population = newGeneration;

        /*************/
        /* Save best */
        /*************/
        if (champion.fitness < encumbment.fitness)
        {
            encumbment = champion;
            memcpy(encumbment_path, champion.path, nnodes * sizeof(int));
        }
        printf("Champion fitness = %lf\n", champion.fitness);
    }

    memcpy(path, population[indexChampion].path, nnodes * sizeof(int));
    *tour_length = population[indexChampion].fitness;

    INFO_COMMENT("heuristic::genetic_algorithm", "Genetic algorithm completed. Best tour length: %lf", *tour_length);

    for (int i = 0; i < populationSize; i++)
    {
        free(population[i].path);
    }
    path = encumbment_path;
    free(population);
}
