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
    return currentIteration > 200;
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
    int tabuNodes[2] = {-1, -1};
    double cost_old_edge, cost_new_edge, cost_old_edge2, cost_new_edge2;
    int encumbment_path[nnodes];
    memcpy(encumbment_path, path, nnodes * sizeof(int));
    double encumbment_tour_length = *tour_length;
    while (!stoppingCondition(currentIteration))
    {
        currentIteration++;
        DEBUG_COMMENT("tabu_search", "iteration: %d", currentIteration);
        min_increase = INFTY;
        foundImprovement = 0;
        pendingTourLenght = *tour_length;
        for (int i = 0; i < nnodes - 2; i++)
        {
            for (int j = i + 1; j < nnodes; j++)
            {
                int j1 = (j + 1) % nnodes;
                cost_old_edge = distance_matrix[path[i] * nnodes + path[i + 1]];
                cost_new_edge = distance_matrix[path[i] * nnodes + path[j]];
                cost_old_edge2 = distance_matrix[path[j] * nnodes + path[j1]];
                cost_new_edge2 = distance_matrix[path[i + 1] * nnodes + path[j1]];
                double delta = (cost_new_edge + cost_new_edge2) - (cost_old_edge + cost_old_edge2);

                if (*tour_length + delta < *tour_length)
                {
                    // not sure about this logic
                    foundImprovement = 1;
                    improvementOperation[0] = i;
                    improvementOperation[1] = j;
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

int compare_individuals(const void *a, const void *b)
{
    const Individual *ind_a = (const Individual *)a;
    const Individual *ind_b = (const Individual *)b;
    if (ind_a->fitness < ind_b->fitness)
        return -1;
    if (ind_a->fitness > ind_b->fitness)
        return 1;
    return 0;
}

int compare_ints(const void *a, const void *b)
{
    int int_a = *((int *)a);
    int int_b = *((int *)b);
    return int_a - int_b;
}

static double getFitness(const double *distance_matrix, int *path, int nnodes)
{
    if (!feasiblePath(path, nnodes))
        ERROR_COMMENT("heuristic.c:getFitness", "path not feasible");
    double fitness = get_tour_length(path, nnodes, distance_matrix);
    return fitness;
}

static int getIndexChampion(Individual population[], int populationSize, int nnodes)
{
    double min = population[0].fitness;
    int indexChampion = 0;
    for (int i = 1; i < populationSize; i++)
    {
        if (population[i].fitness < min)
        {
            min = population[i].fitness;
            indexChampion = i;
        }
    }
    return indexChampion;
}

void longest_common_subpath(const int *nnodes, const int *tour1, const int *tour2, int *length, int *subpath, int *reverse, int *max_start1, int *max_start2)
{
    int max_length = 0;
    *max_start1 = 0;
    *max_start2 = 0;
    *reverse = 0;

    for (int i = 0; i < *nnodes; i++)
    {
        int current_length = 0;
        for (int j = 0; j < *nnodes; j++)
        {
            int idx1 = (i + j) % *nnodes;
            int idx2 = j;

            if (tour1[idx1] == tour2[idx2])
            {
                current_length++;
                if (current_length > max_length)
                {
                    max_length = current_length;
                    *max_start1 = idx1 - current_length + 1;
                    *max_start2 = idx2 - current_length + 1;
                    *reverse = 0;
                }
            }
            else
            {
                current_length = 0;
            }
        }
    }

    for (int i = 0; i < *nnodes; i++)
    {
        int current_length = 0;
        for (int j = 0; j < *nnodes; j++)
        {
            int idx1 = (i + j) % *nnodes;
            int idx2 = (*nnodes - j) % *nnodes;

            if (tour1[idx1] == tour2[idx2])
            {
                current_length++;
                if (current_length > max_length)
                {
                    max_length = current_length;
                    *max_start1 = idx1 - current_length + 1;
                    *max_start2 = idx2 - current_length + 1;
                    *reverse = 1;
                }
            }
            else
            {
                current_length = 0;
            }
        }
    }

    // if (reverse) {
    //     for (int i = 0; i < max_length; i++) {
    //         int idx2 = (*max_start2 + i) % nnodes;
    //         subpath[i] = tour2[idx2];
    //     }
    // } else {
    //     memcpy(subpath, &tour2[*max_start2], max_length * sizeof(int));
    // }

    *length = max_length;
}

void genetic_simple_merge(int *child, const int *p1, const int *p2, const int *nnodes)
{
    int lenChild = randomBetween((int)(*nnodes / 2), (int)(*nnodes * (2 / 3)));
    int *isInChild = calloc(*nnodes, sizeof(int));

    for (int i = 0; i < lenChild; i++)
    {
        child[i] = p1[i];
        isInChild[p1[i]] = 1;
    }

    int pos2 = 0, last = child[lenChild - 1];
    while (p2[pos2] != last)
        pos2++;
    while (pos2 < *nnodes)
    {
        if (!isInChild[p2[pos2]])
        {
            child[lenChild] = p2[pos2];
            lenChild++;
        }
        pos2++;
    }
    pos2 = 0;
    while (lenChild < *nnodes)
    {
        if (!isInChild[p2[pos2]])
        {
            child[lenChild] = p2[pos2];
            lenChild++;
        }
        pos2++;
    }
}

void genetic_new_merge(const int *p1, const int *p2, int *newPath, const int *nnodes)
{

    // Get longhest common subpath
    int reverse = 0, lensubpath;
    int *subpath = malloc(*nnodes * sizeof(int));
    int pos1 = 0, pos2 = 0;
    longest_common_subpath(nnodes, p1, p2, &lensubpath, subpath, &reverse, &pos1, &pos2);

    // find first position longestSubpath
    int lenNewPath = 0;
    for (; p1[pos1] != subpath[0] && p2[pos2] != subpath[0]; pos1++, pos2++)
        ;
    for (; p1[pos1] != subpath[0]; pos1++)
        ;
    for (; p2[pos2] != subpath[0]; pos2++)
        ;

    // copy big portion of parent 1 (champion)
    int *isMissing = calloc(*nnodes, sizeof(int));
    int partOfP1 = randomBetween((int)(*nnodes / 2), (int)(*nnodes * 2 / 3));

    while (lenNewPath < partOfP1)
    {
        newPath[++lenNewPath] = p1[pos1 % *nnodes];
        isMissing[p1[pos1] % *nnodes] = 1;
        pos1++;
    }

    // take (order) remaining nodes from parent 2
    if (!reverse)
    {
        pos2 += lenNewPath;
        while (lenNewPath != *nnodes)
        {
            if (isMissing[p2[pos2 % *nnodes]] == 0)
            {
                newPath[++lenNewPath] = p2[pos2 % *nnodes];
            }
            pos2++;
        }
    }
    else
    {
        pos2 -= lenNewPath;
        while (lenNewPath != *nnodes)
        {
            if (isMissing[p2[pos2 % *nnodes]] == 0)
            {
                newPath[++lenNewPath] = p2[pos2 % *nnodes];
            }
            pos2 = pos2 + *nnodes - 1;
            // if(pos2==-1)pos2=*nnodes-1;
        }
    }
    free(subpath);
    free(isMissing);
}

/**
 * Partially Mapped Crossover (PMX)
 * Order Crossover (OX)
 * Cycle Crossover (CX)
 * Edge Recombination Crossover (ERX)
 * Edge Assembly Crossover (EAX)
 * Sequential Constructive Crossover (SCX)
 * Sub-tour Exchange Crossover (SPEX)
 * Greedy Crossover (GX)
 * Path-Consistency Crossover (PCC)
 */

static int genetic_merge_parents(Individual *child, const Individual *p1, const Individual *p2, const int nnodes, int prob_deletion)
{
    // SETUP
    int countNodes = 0;
    int *isMissing = malloc(nnodes * sizeof(int));
    for (int i = 0; i < nnodes; i++)
    {
        child->path[i] = -1;
        isMissing[i] = 1;
    }
    // which portion of port 1 i will use?
    // int portion1 = (int)(nnodes * (4/10) ) + rand() % (int)(nnodes * (2/10)+1); // portion between quarters
    int portion1 = randomBetween((int)(nnodes * (7 / 10)), nnodes - 1); // portion between quarters

    DEBUG_COMMENT("genetic_merge_parents", "portion1: %d", portion1);

    // START MERGING
    for (int i = 0; i < portion1; i++)
    {
        // if (rand() % 100 < prob_deletion)
        //     continue;
        child->path[countNodes] = p1->path[i];
        isMissing[p1->path[i]] = 0;
        countNodes++;
        DEBUG_COMMENT("genetic_merge_parents", "adding node %d", p1->path[i]);
    }

    for (int j = 0; j < nnodes; j++)
    {
        int nodeToAdd = p2->path[j]; // FIXME start adding from the second portion of the path
        if (!isMissing[nodeToAdd])   // && rand() % 100 < prob_deletion){
        {
            DEBUG_COMMENT("genetic_merge_parents", "already done node %d", nodeToAdd);
            continue;
        }
        child->path[countNodes] = nodeToAdd;
        DEBUG_COMMENT("genetic_merge_parents", "adding node %d", nodeToAdd);
        isMissing[nodeToAdd] = 0;
        countNodes++;
    }

    int count = 0;
    for (int j = 0; j < nnodes; j++)
    {
        if (isMissing[j])
        {
            child->path[countNodes] = j;
            countNodes++;
            count++;
        }
    }
    free(isMissing);
    return count;
}

void genetic_repair_child(Individual *child, const double *distance_matrix, int nnodes, int effort)
{
    two_opt(distance_matrix, nnodes, child->path, &child->fitness, effort);
}

void genetic_add_mutations(Individual *child, int nnodes, int prob_mutation)
{
    int p1, p2, tmp;
    int num_mut = randomBetween(1, nnodes / 2);
    for (int i = 0; i < num_mut; i++)
    {
        p1 = rand() % nnodes;
        do
        {
            p2 = rand() % nnodes;
        } while (p1 == p2);
        tmp = child->path[p1];
        child->path[p1] = child->path[p2];
        child->path[p2] = tmp;
    }
}

double normalizeFitness(double fitness, double maxFitness, double minFitness)
{
    return (fitness - minFitness) / (maxFitness - minFitness) * 100;
}

void genetic_algorithm(const double *distance_matrix, int *path, int nnodes, double *tour_length, int populationSize, int iterations)
{
    INFO_COMMENT("heuristic::genetic_algorithm", "Starting genetic algorithm with population size %d and %d iterations", populationSize, iterations);
    char *tmp_string;
    int prob_deletion = 5;
    int prob_mutation = 5;
    // int newGenerationDimension = populationSize * 1.3;
    int newGenerationDimension = populationSize;
    int pps;
    // Generate random population
    Individual *population = malloc(populationSize * sizeof(Individual));
    for (int i = 0; i < populationSize; i++)
    {
        // population[i].path = malloc(nnodes * sizeof(int));
        population[i].path = malloc(nnodes * sizeof(int));
        generate_random_path(population[i].path, nnodes);
        population[i].fitness = getFitness(distance_matrix, population[i].path, nnodes);
        DEBUG_COMMENT("heuristic::genetic_algorithm", "Individual %d", i);
#ifndef PRODUCTIOn
        tmp_string = getPath(population[i].path, nnodes);
        DEBUG_COMMENT("heuristic::genetic_algorithm", "path: %s", tmp_string);
        free(tmp_string);
#endif
        DEBUG_COMMENT("heuristic::genetic_algorithm", "fitness: %lf", population[i].fitness);
    }

    INFO_COMMENT("heuristic::genetic_algorithm", "Generated random population");

    /** Search champion **/
    Individual champion;
    int indexChampion = getIndexChampion(population, populationSize, nnodes);
    champion.fitness = population[indexChampion].fitness;
    champion.path = malloc(nnodes * sizeof(int));
    memcpy(champion.path, population[indexChampion].path, nnodes * sizeof(int));

    /** ENCUMBENT **/
    Individual encumbment;
    encumbment.fitness = champion.fitness;
    encumbment.path = malloc(nnodes * sizeof(int));
    memcpy(encumbment.path, champion.path, nnodes * sizeof(int));

    /***********************************/
    /* Start generating new population */
    /***********************************/
    Individual *newGeneration;
    Individual child;
    child.path = malloc(nnodes * sizeof(int));

    double *cumulativeProbabilityRemove = malloc(newGenerationDimension * sizeof(double));
    double totalCumulative = 0;
    for (int i = 0; i < newGenerationDimension; cumulativeProbabilityRemove[i] = 1 / (i + 1), totalCumulative += 1 / (i + 1), i++)
        ;
    // double prob_selection;

    int indexParent1, indexParent2;
    int effort;
    double minFitnessGeneration, maxFitnessGeneration;
    for (int iter = 0; iter < iterations; iter++)
    {
        newGeneration = malloc(newGenerationDimension * sizeof(Individual));
        pps = newGenerationDimension;
        printf("Iteration %d\n", iter);
        INFO_COMMENT("heuristic::genetic_algorithm", "Iteration %d", iter);

        minFitnessGeneration = champion.fitness;
        maxFitnessGeneration = 0;

        int probChoseChampion = 70;

        /**START GENERATION**/
        for (int i = 0; i < pps; i++)
        {
            /******************/
            /* Select Parents */
            /******************/
            indexParent1 = rand() % 100 > probChoseChampion ? rand() % populationSize : indexChampion;
            indexParent2 = rand() % populationSize;
            while (indexParent1 == indexParent2)
            {
                indexParent2 = rand() % populationSize;
            }
            DEBUG_COMMENT("heuristic::genetic_algorithm", "Selected parents %d and %d", indexParent1, indexParent2);

#ifdef VERBOSE
            /* print parent's paths*/
            tmp_string = getPath(population[indexParent1].path, nnodes);
            DEBUG_COMMENT("heuristic::genetic_algorithm", "Path p1: %s", tmp_string);
            free(tmp_string);
            tmp_string = getPath(population[indexParent2].path, nnodes);
            DEBUG_COMMENT("heuristic::genetic_algorithm", "Path p2: %s", tmp_string);
            free(tmp_string);
#endif
            /*****************/
            /* Merge Parents */
            /*****************/
            // TODO search a good crossover procedure
            // effort = genetic_merge_parents(&child, &population[indexParent1], &population[indexParent2], nnodes, prob_deletion);
            genetic_simple_merge(child.path, population[indexParent1].path, population[indexParent2].path, &nnodes);
            effort = nnodes * 4;

#ifdef VERBOSE
            tmp_string = getPath(child.path, nnodes);
            DEBUG_COMMENT("heuristic::genetic_algorithm", "merged\t%s", tmp_string);
            free(tmp_string);
#endif
            /**********/
            /* Repair */
            /**********/
            // TODO we need to repair? how?
            // genetic_repair_child(&child, distance_matrix, nnodes, INFINITY);

#ifdef VERBOSE
            tmp_string = getPath(child.path, nnodes);
            DEBUG_COMMENT("heuristic::genetic_algorithm", "repaired\t%s", tmp_string);
            free(tmp_string);
#endif

            /*****************/
            /* Add mutations */
            /*****************/
            // TODO we need to add mutation or we just take that kind of crossover as enough mutations
            // if(iter<iterations-1)
            // genetic_add_mutations(&child, nnodes, prob_mutation);

            /***************/
            /* Store child */
            /***************/
            child.fitness = getFitness(distance_matrix, child.path, nnodes);
            // print fitness and path
            DEBUG_COMMENT("heuristic::genetic_algorithm", "Child %d, fitness %lf", i, child.fitness);
            tmp_string = getPath(child.path, nnodes);
            DEBUG_COMMENT("heuristic::genetic_algorithm", "path: %s", tmp_string);
            free(tmp_string);

            newGeneration[i].path = malloc(nnodes * sizeof(int));
            memcpy(newGeneration[i].path, child.path, nnodes * sizeof(int));
            newGeneration[i].fitness = child.fitness;

            /*****************/
            /* Find champion */
            /*****************/
            if (newGeneration[i].fitness < minFitnessGeneration)
            {
                minFitnessGeneration = newGeneration[i].fitness;
                indexChampion = i;
            }
            if (newGeneration[i].fitness > maxFitnessGeneration)
            {
                maxFitnessGeneration = newGeneration[i].fitness;
            }
        }
        /**END OF GENERATION ITERATIONS**/

        //         /***************************/
        //         /* Kinda natural selection */
        //         /***************************/
        //         qsort(newGeneration, pps, sizeof(Individual), compare_individuals);
        //         int *indices_selected = malloc((pps - populationSize) * sizeof(int));
        //         int size_indicex = 0;
        //         int index, indRemove_found;

        //         for (int i = 0; i < pps - populationSize; i++)
        //         {
        //             indRemove_found = 0;
        //             do
        //             {
        //                 prob_selection = ((double)rand() / RAND_MAX) * totalCumulative;
        //                 index = 0;
        //                 while (prob_selection > 0)
        //                 {
        //                     prob_selection -= cumulativeProbabilityRemove[index];
        //                     index++;
        //                 }
        //                 index--;

        //                 for (int j = 0; j < size_indicex; j++)
        //                 {
        //                     if (indices_selected[j] == index)
        //                     {
        //                         indRemove_found = 1;
        //                         break;
        //                     }
        //                 }
        //             } while (indRemove_found);
        //             indices_selected[size_indicex++] = index;
        //         }

        //         qsort(indices_selected, size_indicex, sizeof(int), compare_ints);
        // #ifndef PRODUCTION
        //         tmp_string = getPath(indices_selected, size_indicex);
        //         DEBUG_COMMENT("heurisict.c::genetic_algorithm", "indices selected: %s", tmp_string);
        //         free(tmp_string);
        // #endif
        //         return;
        //         for (int i = size_indicex; i > 0; i--)
        //         {
        //         }

        /*******************************************/
        /* Store new generation, memory management */
        /*******************************************/
        /* Champion */
        // OUTPUT_COMMENT("heuristic::genetic_algorithm", "Champion of iteration %d has fitness %lf", iter, minFitnessGeneration);
        champion.fitness = minFitnessGeneration;
        memcpy(champion.path, newGeneration[indexChampion].path, nnodes * sizeof(int));

        /* New generation and remove Old */
        for (int i = 0; i < populationSize; i++)
        {
            free(population[i].path);
        }
        free(population);
        population = newGeneration;

        /*************/
        /* Save best */
        /*************/
        if (champion.fitness >= encumbment.fitness)
            break;

        encumbment.fitness = champion.fitness;
        memcpy(encumbment.path, champion.path, nnodes * sizeof(int));

        INFO_COMMENT("heuristic::genetic_algorithm", "New encumbment found with fitness %lf", encumbment.fitness);
        tmp_string = getPath(encumbment.path, nnodes);
        INFO_COMMENT("heuristic::genetic_algorithm", "path: %s", tmp_string);
        free(tmp_string);
    }

    /***********/
    /* Wrap up */
    /***********/
    /* Save results */
    memcpy(path, encumbment.path, nnodes * sizeof(int));
    *tour_length = encumbment.fitness;

    INFO_COMMENT("heuristic::genetic_algorithm", "Genetic algorithm completed. Best tour length: %lf", *tour_length);

    /* Free memory */
    for (int i = 0; i < populationSize; i++)
    {
        free(population[i].path);
        // free(newGeneration[i].path);
    }

    free(child.path);
    free(population);
    // free(newGeneration);
}