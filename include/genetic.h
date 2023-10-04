#ifndef GENETIC_H
#define GENETIC_H
#include "grasp.h"

typedef struct {
    int *path;
    int nnodes;
    int fitness;
} GENETIC_INDIVIDUAL;

typedef struct {
    GENETIC_INDIVIDUAL *individual;
    int population_size;

    GRASP_Framework *grasp_individuals;
} GENETIC_POPULATION;

typedef struct {
    // setup
    int ngenerations;
    int population_size;
    int windows_size;
    int grasp_n_probabilities;
    double *grasp_probabilities;

    // data
    GENETIC_POPULATION *parents, *children;

    //int crossover_rate;
    //int mutation_rate;
    GENETIC_INDIVIDUAL *best_individual;
} GENETIC_SETUP;

void genetic_setup(GENETIC_SETUP *setup,
        int ngenerations,
        int population_size,
        int windows_size,
        int grasp_n_probabilities,
        double *grasp_probabilities);

void genetic_population(GENETIC_POPULATION *population,
        GENETIC_SETUP *setup
        );

void genetic_individual(GENETIC_INDIVIDUAL *individual, int nnodes, int *path, double tour_length);

void genetic_destroy(GENETIC_SETUP *setup);

void genetic_destroy_population(GENETIC_POPULATION *population);


#endif // !GENETIC_H
