#include "../include/heuristic.h"
#include "../include/refinement.h"
#include <math.h>



void refine_pop(Instance *inst, GENETIC_POPULATION *population, time_t duration){

    for(int i=0; i < population->population_size; i++){
        Instance *I = temp_instance(inst, population->individual[i].path);
        // I->x = inst->x;
        // I->y = inst->y;
        // I->nnodes = inst->nnodes;
        // I->distance_matrix = inst->distance_matrix;
        // I->path = population->individual[i].path;
        // I->tour_length = population->individual[i].fitness;
        // I->tstart = time(NULL);
        // I->tend = I->tstart + duration;
        // // I->max_time = I->tend;
        I->tend = time(NULL) + duration;
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

            if (ind1->path[i] == ind2->path[p])
                matches_fw[i] = 1;
            else if(ind1->path[i] == ind2->path[m])
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
    DEBUG_COMMENT("heuristic::crossover", "Finding candidates");

    int rnd;
    for(int i=0; i<gen->population_size; i++){
        int ch=get_solution(gen->parents->grasp_individuals);
        GENETIC_INDIVIDUAL* champion = &gen->parents->individual[ch];
        DEBUG_COMMENT("heuristic::crossover", "Finding candidates for champion %d -> %d", i, ch);

        do{
            rnd = rand() % gen->population_size;
        }while(inGrasp(gen->parents->grasp_individuals, rnd));

        GENETIC_INDIVIDUAL* chosen = &gen->parents->individual[rnd];
        DEBUG_COMMENT("heuristic::crossover", "Chosen %d", rnd);

        int s1=0, s2=0;
        DEBUG_COMMENT("heuristic::crossover", "Finding starting nodes %d", inst->starting_node);
        for(; champion->path[s1] != inst->starting_node; s1++);
        for(; chosen->path[s2] != inst->starting_node; s2++);

        DEBUG_COMMENT("heuristic::crossover", "Starting crossover");
        double prob_cross[] = {0.65, 0.35};
        int size = 0;
        int *inserted = calloc(inst->nnodes, sizeof(int));

        GENETIC_INDIVIDUAL child;
        child.path = malloc(sizeof(int) * inst->nnodes);
        child.fitness = 0;
        child.nnodes = inst->nnodes;

        int i1=0, i2=0;
        int p1 = s1 + i1;
        int p2 = s2 + i2;
        child.path[size] = champion->path[p1];
        inserted[champion->path[p1]] = 1;
        i1++; i2++; size++;
        p1 = (s1 + i1) % child.nnodes;
        p2 = (s2 + i2) % child.nnodes;

        while(size < inst->nnodes && i1 < inst->nnodes && i2 < inst->nnodes){
            for(; inserted[champion->path[p1]] == 1 && i1 < inst->nnodes; i1++) p1 = (s1 + i1) % child.nnodes;
            for(; inserted[chosen->path[p2]] == 1 && i2 < inst->nnodes; i2++) p2 = (s2 + i2) % child.nnodes;

            if (rand() % 100 < prob_cross[0] * 100){
                child.path[size] = champion->path[p1];
                inserted[champion->path[p1]] = 1;
                i1++; p1 = (s1 + i1) % child.nnodes;
            } else {
                child.path[size] = chosen->path[p2];
                inserted[chosen->path[p2]] = 1;
                i2++; p2 = (s2 + i2) % child.nnodes;
            }
            size++;
        }
        DEBUG_COMMENT("heuristic::crossover", "Exit first while");
        DEBUG_COMMENT("heuristic::crossover", "Child generated");
        DEBUG_COMMENT("heuristic::crossover", "size is %d", size);
        DEBUG_COMMENT("heuristic::crossover", "i1 is %d", i1);
        DEBUG_COMMENT("heuristic::crossover", "i2 is %d", i2);
        DEBUG_COMMENT("heuristic::crossover", "nnodes are %d", inst->nnodes);

        while(size < inst->nnodes && i1 < inst->nnodes){
            for(; inserted[champion->path[p1]] == 1 && i1 < inst->nnodes; i1++) p1=(s1 + i1) % child.nnodes;
            if (i1 > inst->nnodes)
                break;
            DEBUG_COMMENT("heuristic::crossover", "Inserting s1 %d,  i1 %d , p1=%d", s1, i1, p1);
            child.path[size] = champion->path[p1];
            inserted[champion->path[p1]] = 1;
            i1++; p1 = (i1 + s1) % child.nnodes;
            size++;
        }
        DEBUG_COMMENT("heuristic::crossover", "Exit second while");
        DEBUG_COMMENT("heuristic::crossover", "Child generated");
        DEBUG_COMMENT("heuristic::crossover", "size is %d", size);
        DEBUG_COMMENT("heuristic::crossover", "i1 is %d", i1);
        DEBUG_COMMENT("heuristic::crossover", "i2 is %d", i2);
        DEBUG_COMMENT("heuristic::crossover", "nnodes are %d", inst->nnodes);

        while(size < inst->nnodes && i2 < inst->nnodes){
            for(; inserted[chosen->path[p2]] == 1 && i2 < inst->nnodes; i2++)p2=(s2 + i2) % child.nnodes;
            if (i2 > inst->nnodes)
                break;
            child.path[size] = chosen->path[p2];
            inserted[chosen->path[p2]] = 1;
            i2++; p2 = (i2 + s2) % child.nnodes;
            size++;
        }

        DEBUG_COMMENT("heuristic::crossover", "Exit third while");


        DEBUG_COMMENT("heuristic::crossover", "Child generated");
        DEBUG_COMMENT("heuristic::crossover", "size is %d", size);
        DEBUG_COMMENT("heuristic::crossover", "i1 is %d", i1);
        DEBUG_COMMENT("heuristic::crossover", "i2 is %d", i2);
        DEBUG_COMMENT("heuristic::crossover", "nnodes are %d", inst->nnodes);
        if(size != inst->nnodes){
            ERROR_COMMENT("heuristic::crossover", "size is not nnodes");
            exit(-1);
        }
        if(i1 < inst->nnodes){
            ERROR_COMMENT("heuristic::crossover", "i1 is not nnodes");
            exit(-1);
        }
        if(i2 < inst->nnodes){
            ERROR_COMMENT("heuristic::crossover", "i2 is not nnodes");
            exit(-1);
        }

        child.fitness = calculateTourLenghtPath(inst, child.path);
        add_solution(gen->children->grasp_individuals, i, child.fitness);

        // Copy the child to the population and free the allocated memory
        gen->children->individual[i] = child;
        free(inserted);
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
    DEBUG_COMMENT("heuristic::genetic_algorithm", "Starting genetic algorithm");

    // BASIC SETUP
    // grasp size 6
    int n_prob=6;
    double *temp_prob=malloc(sizeof(double)*n_prob);
    for (int i=1; i <= n_prob; i++){
        if (i==1)
            temp_prob[i-1] = 0.5;
        else
            temp_prob[i-1] = (double)(1/pow(2,i))+temp_prob[i-2];
        DEBUG_COMMENT("heuristic::genetic_algorithm", "Grasp %d: %f", i, temp_prob[i-1]);
    }
    GENETIC_SETUP *gen = inst->genetic_setup;
    genetic_setup(gen, 1000, 100, (int)(inst->nnodes / 10) , n_prob, temp_prob);
    // set best individual
    gen->best_individual=malloc(sizeof(GENETIC_INDIVIDUAL));
    genetic_individual(gen->best_individual, inst->nnodes, NULL, INFINITY);
    DEBUG_COMMENT("heuristic::genetic_algorithm", "Best individual set");
    temp_prob=NULL;

    // INIT FIRST GENERATION
    gen->parents=malloc(sizeof(GENETIC_POPULATION));
    genetic_population(gen->parents, gen);
    DEBUG_COMMENT("heuristic::genetic_algorithm", "Generating first population");
    for(int i=0;i<gen->population_size;i++){
        int *tpath = malloc(sizeof(int)*inst->nnodes);
        double ttl = generate_random_path(inst, tpath);
        genetic_individual(
                &gen->parents->individual[i],
                inst->nnodes,
                tpath,
                ttl
        );
        //add_solution(gen->parents->grasp_individuals, i, gen->parents->individual[i].fitness);
    }

    // QUICK REFINEMENT
    DEBUG_COMMENT("heuristic::genetic_algorithm", "Refining population");
    refine_pop(inst, gen->parents, 3);
    for (int i=0; i<gen->population_size; i++){
        add_solution(gen->parents->grasp_individuals, i, gen->parents->individual[i].fitness);
    }
    // DEBUG COMMENT grasp
    DEBUG_COMMENT("heuristic::genetic_algorithm", "GRAPPA");
    for (int i=0; i<gen->grasp_n_probabilities; i++){
        DEBUG_COMMENT("heuristic::genetic_algorithm", "Grasp %d - %d: %f", i,gen->parents->grasp_individuals->solutions[i].solution , gen->parents->grasp_individuals->solutions[i].value);
    }

    for(int iter=0; iter < gen->ngenerations; iter++){
        DEBUG_COMMENT("heuristic::genetic_algorithm", "Starting generation %d", iter);
        if(gen->children){
            ERROR_COMMENT("heuristic::genetic_algorithm", "children is not NULL");
            exit(-1);
        }
        gen->children = malloc(sizeof(GENETIC_POPULATION));
        genetic_population(gen->children, gen);

        // GENERATE CHILDRENS
        DEBUG_COMMENT("heuristic::genetic_algorithm", "Crossover");
        crossover(inst, gen);

        // refine children
        DEBUG_COMMENT("heuristic::genetic_algorithm", "Refining children");
        refine_pop(inst, gen->children, 3);

        DEBUG_COMMENT("heuristic::genetic_algorithm", "Selecting best children");
        genetic_destroy_population(gen->parents);

        gen->parents = gen->children;
    }

    DEBUG_COMMENT("heuristic::genetic_algorithm", "Selecting best solution");

    genetic_destroy_population(gen->parents);
    genetic_destroy_population(gen->children);
    return SUCCESS;
}
