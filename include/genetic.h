#ifndef GENETIC_H
#define GENETIC_H
#include "grasp.h"

typedef struct {
    int *chromosome;
    int fitness;
} GENETIC_INDIVIDUAL;

typedef struct {
    GENETIC_INDIVIDUAL *individual;
    int size;

    GRASP_Framework *grasp_individuals;
    int best_fitness;
    int best_individual;
} GENETIC_POPULATION;

typedef struct {
    // setup
    int generations;

    // data
    GENETIC_POPULATION *parents, *children;
    int population_size;
    double *grasp_probabilities;
    int grasp_n_probabilities;

    // crossover
    int windows_size;
    int crossover_rate;
    int mutation_rate;
} GENETIC_SETUP;

#endif // !GENETIC_H
