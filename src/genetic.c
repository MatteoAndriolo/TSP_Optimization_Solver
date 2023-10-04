#include "../include/genetic.h"
#include <stdlib.h>
#include "../include/logger.h"

void genetic_setup(GENETIC_SETUP *gen,
        int ngenerations,
        int population_size,
        int windows_size,
        int grasp_n_probabilities,
        double *grasp_probabilities){
    if (!gen){
        ERROR_COMMENT("genetic_setup", "gen is NULL");
        exit(-1);
    }
    gen->ngenerations = ngenerations;
    gen->population_size = population_size;
    gen->windows_size=windows_size;
    gen->grasp_probabilities=grasp_probabilities;
    gen->grasp_n_probabilities=grasp_n_probabilities;
}

void genetic_population(GENETIC_POPULATION *popul,
        GENETIC_SETUP *gen
        ){
    if (!popul){
        ERROR_COMMENT("genetic_population", "popul is NULL");
        exit(-1);
    }
    popul->individual = malloc(gen->population_size* sizeof(GENETIC_INDIVIDUAL));
    popul->population_size = gen->population_size;
    popul->grasp_individuals = malloc(sizeof(GRASP_Framework));
    init_grasp(popul->grasp_individuals, gen->grasp_probabilities,gen->grasp_n_probabilities);
}

void genetic_individual(GENETIC_INDIVIDUAL *indiv,
        int nnodes,
        int *path,
        double tour_lenght){
    if (!indiv){
        ERROR_COMMENT("genetic_individual", "indiv is NULL");
        exit(-1);
    }
    indiv->nnodes = nnodes;
    indiv->fitness= tour_lenght;
    if(!path)
        indiv->path = malloc(sizeof(int)*nnodes);
    else
        indiv->path = path;
}

void genetic_destroy_population(GENETIC_POPULATION *popul){
    if (!popul){
        ERROR_COMMENT("genetic_destroy_population", "popul is NULL");
        exit(-1);
    }
    free_grasp(popul -> grasp_individuals);
    for(int i=0;i<popul->population_size;i++){
        free(popul->individual[i].path);
        popul->individual[i].path=NULL;
    }
    free(popul->individual);
    popul->individual=NULL;
    free(popul);
    popul=NULL;
}

void genetic_destroy(GENETIC_SETUP *gen){
    if (!gen)
        return;
    genetic_destroy_population(gen->parents);
    genetic_destroy_population(gen->children);
    if (!gen->grasp_probabilities){
        free(gen->grasp_probabilities);
        gen->grasp_probabilities=NULL;
    }
    free(gen);
}

