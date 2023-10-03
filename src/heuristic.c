#include "../include/heuristic.h"
#include "../include/refinement.h"
#include <math.h>


int* generate_random_path(Instance *inst){
    // copy of instance_generate_path
    int *path = malloc(inst->nnodes * sizeof(int));
    int random_start = rand() % inst->nnodes;
    for (int i = 0; i < inst->nnodes; i++) {
        path[i] = i;
    }
    path[random_start] = 0;
    path[0] = random_start;

    int j, tmp;
    for (int i = 0; i < inst->nnodes; i++) {
        j = rand() % inst->nnodes;
        tmp = path[i];
        path[i] = path[j];
        path[j] = tmp;
    }
    return path;
}

void destroy_population(GENETIC_POPULATION *population){
    for(int i=0; i < population->size; i++){
        FREE(population->individual[i].chromosome);
    }
    free_grasp(population->grasp_individuals);
    FREE(population->individual);
    FREE(population);
}

void init_population(Instance *inst, GENETIC_POPULATION *population){
    if (population->individual != NULL){
        destroy_population(population);
    }
    population = (GENETIC_POPULATION*) malloc(sizeof(GENETIC_POPULATION));
    population->best_fitness = 1.0/0.0; // INFINITY
    population->best_individual = -1;
    population->size = inst->genetic_setup.population_size;
    population->individual = (GENETIC_INDIVIDUAL*) malloc(sizeof(GENETIC_INDIVIDUAL) * population->size);
    for(int i=0; i<population->size ; i++){
        GENETIC_INDIVIDUAL *ind = malloc(sizeof(GENETIC_INDIVIDUAL));
        ind->chromosome = generate_random_path(inst);
        ind->fitness = calculateTourLength(inst);
        population->individual[i] = *ind;
    }
    init_grasp(population->grasp_individuals, inst->genetic_setup.grasp_probabilities, inst->genetic_setup.grasp_n_probabilities);
}

void refine_pop(Instance *inst, GENETIC_POPULATION *population, time_t duration){
    for(int i=0; i < population->size; i++){
        Instance *I = malloc(sizeof(Instance));
        I->x = inst->x;
        I->y = inst->y;
        I->nnodes = inst->nnodes;
        I->distance_matrix = inst->distance_matrix;
        I->path = population->individual[i].chromosome;
        I->tour_length = population->individual[i].fitness;
        I->tstart = time(NULL);
        I->tend = I->tstart + duration;
        // I->max_time = I->tend;
        two_opt(I, INFINITY);


        population->individual[i].fitness = I->tour_length;
        // TODO GRASP
        // if(I->tour_length < population->best_fitness){
        //     population->best_fitness = I->tour_length;
        //     population->best_individual = i;
        // }
        add_solution(inst->grasp, i, I->tour_length);
        FREE(I);
    }
}


int* matches(Instance *inst, GENETIC_INDIVIDUAL *ind1, GENETIC_INDIVIDUAL *ind2){
    int windows_size = (int) inst->nnodes/10;
    int *matches_fw = (int*) calloc(inst->nnodes*2, sizeof(int));
    int *matches_bw = matches_fw + inst->nnodes;

    for (int i=0; i<inst->nnodes; i++){
        for (int j=0; j< (int)windows_size/2; j++){
            int p=(i+j)%inst->nnodes;
            int m=i-j;
            if (m<0){
                m=inst->nnodes+m;
            }

            if (ind1->chromosome[i] == ind2->chromosome[p])
                matches_fw[i] = 1;
            else if(ind1->chromosome[i] == ind2->chromosome[m])
                matches_bw[i] = 1;
        }
    }
    return matches_fw;
}

int allineate_paths(GENETIC_SETUP* gen, GENETIC_INDIVIDUAL *ind1, GENETIC_INDIVIDUAL *ind2, int * matches){
    int* matches_fw = matches;
    int* matches_bw = matches + gen->population_size;
    int nnodes = gen->population_size;
    int windows_size = gen->windows_size;

    int* cumul_fw = (int*) calloc(nnodes, sizeof(int));
    int* cumul_bw = (int*) calloc(nnodes, sizeof(int));

    int best_fw = 0;
    int best_bw = 0;
    int best_fw_pos = 0;
    int best_bw_pos = 0;


    // init first and last
    for(int i=0; i<windows_size; i++){
        cumul_bw[0]+=matches_bw[gen->population_size - i];
        cumul_fw[gen->population_size-1]+=matches_fw[i];
    }

    if (cumul_bw[0]>best_bw){
        best_bw = cumul_bw[0];
        best_bw_pos = 0;
    }
    if (cumul_fw[gen->population_size-1]>best_fw){
        best_fw = cumul_fw[gen->population_size-1];
        best_fw_pos = gen->population_size-1;
    }

    // do the first portion and last portion which use circularity of array
    for(int i=1; i<windows_size;i++){
        cumul_bw[i]=cumul_bw[i-1]+matches_bw[i]-matches_bw[gen->population_size - windows_size +i];
        cumul_fw[gen->population_size-1-i]=cumul_fw[gen->population_size-i]+matches_fw[windows_size-i]-matches_fw[windows_size-i];

        if (cumul_bw[i]>best_bw){
            best_bw = cumul_bw[i];
            best_bw_pos = i;
        }
        if (cumul_fw[gen->population_size-1-i]>best_fw){
            best_fw = cumul_fw[gen->population_size-1-i];
            best_fw_pos = gen->population_size-1-i;
        }
    }

    // do the rest
    for(int i=windows_size; i<nnodes-windows_size; i++){
        cumul_bw[i]=cumul_bw[i-1]+matches_bw[i]-matches_bw[i-windows_size];
        cumul_fw[nnodes-1-i]=cumul_fw[nnodes-i]+matches_fw[i]-matches_fw[i+windows_size];

        if (cumul_bw[i]>best_bw){
            best_bw = cumul_bw[i];
            best_bw_pos = i;
        }
        if (cumul_fw[nnodes-1-i]>best_fw){
            best_fw = cumul_fw[nnodes-1-i];
            best_fw_pos = nnodes-1-i;
        }
    }

    if(best_fw<best_bw)
        return best_bw_pos;
    return best_fw_pos;
}

void crossover(Instance *inst, GENETIC_SETUP *gen){
    // FIND CANDIDATES FOR EACH CHAMPION
    // GRASP_Framework *candidates = malloc(sizeof(GRASP_Framework)*inst->grasp->size);
    // double *probabilities = malloc(sizeof(double)*inst->grasp->size);
    // for (int i=1; i <= gen->population_size; i++){
    //     probabilities[i] =grasp 1/pow(2,i);
    // }
    // for (int i=0; i<inst->grasp->size; i++){
    //     init_grasp(& candidates[i], probabilities, gen->population_size);
    // }
    gen->windows_size = (int) inst->nnodes/10;

    int rnd;
    for(int i=0; i<gen->population_size; i++){
        const GENETIC_INDIVIDUAL* champion =(const GENETIC_INDIVIDUAL*) &gen->parents[get_solution(gen->parents->grasp_individuals)];
        // get random from population not equal to any of the champions
        do{
            rnd = rand() % gen->population_size;
        }while(inGrasp(inst->grasp, rnd));
        const GENETIC_INDIVIDUAL* chosen =(const GENETIC_INDIVIDUAL*) &gen->parents[rnd];

        // find starting nnodes
        int i1=0, i2=0;
        for(;chosen->chromosome[i1]!=inst->starting_node; i1++);
        for(;champion->chromosome[i2]!=inst->starting_node; i2++);

        double prob_cross[]={0.65, 0.35};
        int size=0;
        int *inserted=calloc(inst->nnodes, sizeof(int));
        GENETIC_INDIVIDUAL *child = malloc(sizeof(GENETIC_INDIVIDUAL));
        child->chromosome = malloc(sizeof(int)*inst->nnodes);
        child->fitness = 0;
        gen->children->individual[i] = *child;
        while(size<inst->nnodes && i1<inst->nnodes &&i2<inst->nnodes){
            for(;inserted[champion->chromosome[i1]]==1 && i1<inst->nnodes; i1++);
            for(;inserted[chosen->chromosome[i2]]==1 && i2<inst->nnodes; i2++);

            if (rand() % 100 < prob_cross[0]*100){
                child->chromosome[size]=champion->chromosome[i1];
                inserted[champion->chromosome[i1]]=1;
                i1++;
            }else{
                child->chromosome[size]=chosen->chromosome[i2];
                inserted[chosen->chromosome[i2]]=1;
                i2++;
            }
            size++;
        }

        while(size<inst->nnodes && i1<inst->nnodes){
            for(;inserted[champion->chromosome[i1]]==1 && i1<inst->nnodes; i1++);
            child->chromosome[size]=champion->chromosome[i1];
            inserted[champion->chromosome[i1]]=1;
            i1++;
            size++;
        }

        while(size<inst->nnodes && i2<inst->nnodes){
            for(;inserted[chosen->chromosome[i2]]==1 && i2<inst->nnodes; i2++);
            child->chromosome[size]=chosen->chromosome[i2];
            inserted[chosen->chromosome[i2]]=1;
            i2++;
            size++;
        }

        child->fitness = calculateTourLenghtPath(inst,child->chromosome);
        add_solution(gen->children->grasp_individuals, i, child->fitness);
    }
}



/*
 *  Start by generating populatin size random paths
 *  Run very briefly 2opt
 *  Allign in some pway paths
 *  Find first few with best best_fitness
 *  Catch a batch of them with highest Jaccard Probability
 *  Crossover them (can do multiple allignments of batches)
 *  Insert in some way also the less important one to give more bariability
 *  Mutations?
 *  Refinement?
 */
int genetic_algorithm(Instance *inst){
    // initiate population
    GENETIC_SETUP *gen = &inst->genetic_setup;
    init_population(inst, gen->parents);

    double *probabilities = malloc(sizeof(double)*gen->population_size);
    for (int i=1; i <= gen->population_size; i++){
        probabilities[i] = 1/pow(2,i);
    }
    gen->grasp_probabilities = probabilities;
    gen->grasp_n_probabilities = gen->population_size;

    init_grasp(gen->parents->grasp_individuals, gen->grasp_probabilities, gen->grasp_n_probabilities);

    // quick Refinement
    refine_pop(inst, gen->parents, 3);

    for(int iter=0; iter < gen->generations; iter++){
        init_population(inst, gen->children);
        // generate children
        crossover(inst, gen);

        // refine children
        refine_pop(inst, gen->children, 3);

        destroy_population(gen->parents);

        gen->parents = gen->children;
    }


    destroy_population(gen->parents);
    destroy_population(gen->children);
    return SUCCESS;
}
