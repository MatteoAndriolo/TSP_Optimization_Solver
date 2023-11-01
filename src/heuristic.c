#include "../include/heuristic.h"
#include "../include/refinement.h"
#include <math.h>
#include <stdio.h>

void refine_pop(Instance *inst, GENETIC_POPULATION *population,
                time_t duration) {

  for (int i = 0; i < population->population_size; i++) {
    Instance *I = temp_instance(inst, population->individual[i].path);
    I->tend = time(NULL) + duration;
    two_opt(I, pow(inst->nnodes, 2));

    population->individual[i].fitness = I->tour_length;
    add_solution(population->grasp_individuals, i, I->tour_length);
    FREE(I);
  }
}

int _insert_node(GENETIC_INDIVIDUAL *src, GENETIC_INDIVIDUAL *dest,
                 bool *inserted, int *pos, int *iter, int *size) {
  int err = SUCCESS;
  if (inserted[src->path[*pos]]) {
    DEBUG_COMMENT("heuristic::crossover", "ERR: already inserted %d",
                  src->path[*pos]);
    err = ERROR;
  } else {
    dest->path[*size] = src->path[*pos];
    DEBUG_COMMENT("heuristic::crossover", "OK : inserted %d",
                  dest->path[*size]);
    (*size)++;
    inserted[src->path[*pos]] = true;
  }
  *pos = (*pos + 1) % dest->nnodes;
  (*iter)++;

  return err;
}

void crossover(Instance *inst, GENETIC_SETUP *gen) {
  DEBUG_COMMENT("heuristic::crossover", "ENTERING FUNCTION");
  DEBUG_COMMENT("heuristic::crossover", "Finding candidates");
  double prob_cross[] = {0.65, 0.35};

  for (int i = 0; i < gen->population_size; i++) {
    int ch = get_solution(gen->parents->grasp_individuals);
    GENETIC_INDIVIDUAL *champion = &gen->parents->individual[ch];
    GENETIC_INDIVIDUAL *chosen;
    int rnd;
    DEBUG_COMMENT("heuristic::crossover",
                  "Finding candidates for champion %d -> %d", i, ch);
    do {
      rnd = rand() % gen->population_size;
    } while (inGrasp(gen->parents->grasp_individuals, rnd));
    chosen = &gen->parents->individual[rnd];

    DEBUG_COMMENT("heuristic::crossover", "Chosen %d", rnd);

    int s1 = 0, s2 = 0;
    DEBUG_COMMENT("heuristic::crossover", "Finding starting nodes %d",
                  inst->starting_node);
    for (; champion->path[s1] != inst->starting_node && s1 < inst->nnodes; s1++)
      ;
    for (; chosen->path[s2] != inst->starting_node && s2 < inst->nnodes; s2++)
      ;
    DEBUG_COMMENT("heuristic::crossover", "Starting nodes found %d %d", s1, s2);

    DEBUG_COMMENT("heuristic::crossover", "Starting crossover");
    int size = 0;
    bool *inserted = malloc(inst->nnodes * sizeof(bool));
    for (int i = 0; i < inst->nnodes; i++) {
      inserted[i] = false;
    }
    if (!inserted) {
      ERROR_COMMENT("heuristic::crossover", "CALLOC ERROR");
      exit(ERROR_ALLOCATION);
    }

    GENETIC_INDIVIDUAL child;
    child.path = malloc(sizeof(int) * inst->nnodes);
    if (!child.path) {
      ERROR_COMMENT("heuristic::crossover", "CALLOC ERROR");
      free(inserted);
      exit(ERROR_ALLOCATION);
    }
    child.fitness = 0;
    child.nnodes = inst->nnodes;

    int i1 = 0, i2 = 0;
    int p1 = s1 + i1;
    int p2 = s2 + i2;
    _insert_node(champion, &child, inserted, &p1, &i1, &size);
    DEBUG_COMMENT("heuristic::crossover", "inserted %d", child.path[size]);
    i2++;
    p2 = (s2 + i2) % child.nnodes;

    bool isChampion = true;
    while (i1 < inst->nnodes || i2 < inst->nnodes) {
      DEBUG_COMMENT("heuristic::crossover", "size %d", size);
      if (i1 >= inst->nnodes || i2 >= inst->nnodes) {
        if (i1 < inst->nnodes) {
          isChampion = true;
        } else if (i2 < inst->nnodes) {
          isChampion = false;
        } else {
          ERROR_COMMENT("heuristic::crossover",
                        "both iterators reached end but size not correct yet");
          exit(-5);
        }
      } else {
        if (rand() % 100 < prob_cross[0] * 100) {
          isChampion = true;
        } else {
          isChampion = false;
        }
      }

      if (isChampion) {
        DEBUG_COMMENT("heuristic::crossover", "Champion");
        while (_insert_node(champion, &child, inserted, &p1, &i1, &size) !=
                   SUCCESS &&
               i1 < inst->nnodes)
          ;
      }

      else {
        DEBUG_COMMENT("heuristic::crossover", "Chosen");
        while (_insert_node(chosen, &child, inserted, &p2, &i2, &size) !=
                   SUCCESS &&
               i2 < inst->nnodes)
          ;
      }
      DEBUG_COMMENT("heuristic::crossover", "size %d", size);
      DEBUG_COMMENT("heuristic::crossover", "i1 %d,i2 %d, p1 %d, p2 %d", i1, i2,
                    p1, p2);
    }

    DEBUG_COMMENT("heuristic::crossover", "Exit FINAL while");

    // ------------------ DEBUG ------------------
    if (size != inst->nnodes) {
      for (int l = 0; l < inst->nnodes; l++)
        if (!inserted)
          DEBUG_COMMENT("heuristic::crossover", "missing node[%d]=%d", l,
                        chosen->path[l]);
      ErrorCode err =
          assert_path(child.path, inst->distance_matrix, inst->nnodes, -1);
      if (err == ERROR_INVALID_PATH)
        for (int l = 0; l < inst->nnodes; l++)
          DEBUG_COMMENT("heuristic::crossover", "errAss path[%d]=%d", l,
                        child.path[l]);
      ERROR_COMMENT("heuristic::crossover", "size is not nnodes");
      exit(-1);
    }
    if (i1 < inst->nnodes) {
      ERROR_COMMENT("heuristic::crossover", "i1 is not nnodes");
      exit(-1);
    }
    if (i2 < inst->nnodes) {
      ERROR_COMMENT("heuristic::crossover", "i2 is not nnodes");
      exit(-1);
    }
    // ------------------ ENDDEBUG ------------------

    child.fitness = calculateTourLenghtPath(inst, child.path);

    gen->children->individual[i] = child;
    free(inserted);
  }

  DEBUG_COMMENT("heuristic::crossover", "EXITING FUNCTION");
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
int genetic_algorithm(Instance *inst) {
  DEBUG_COMMENT("heuristic::genetic_algorithm", "Starting genetic algorithm");

  // BASIC SETUP
  int n_prob = 6;
  double *temp_prob = malloc(sizeof(double) * n_prob);
  for (int i = 1; i <= n_prob; i++) {
    if (i == 1)
      temp_prob[i - 1] = 0.5;
    else
      temp_prob[i - 1] = (double)(1 / pow(2, i)) + temp_prob[i - 2];
    DEBUG_COMMENT("heuristic::genetic_algorithm", "Grasp %d: %f", i,
                  temp_prob[i - 1]);
  }
  GENETIC_SETUP *gen = inst->genetic_setup;
  genetic_setup(gen, inst->ngenerations, inst->population_size,
                (int)(inst->nnodes / 10), n_prob, temp_prob);

  // Set Best Individual
  gen->best_individual = malloc(sizeof(GENETIC_INDIVIDUAL));
  genetic_individual(gen->best_individual, inst->nnodes, NULL, INFINITY);
  DEBUG_COMMENT("heuristic::genetic_algorithm", "Best individual set");
  temp_prob = NULL;

  // INIT FIRST GENERATION
  gen->parents = malloc(sizeof(GENETIC_POPULATION));
  genetic_population(gen->parents, gen);
  DEBUG_COMMENT("heuristic::genetic_algorithm", "Generating first population");
  for (int i = 0; i < gen->population_size; i++) {
    int *tpath = malloc(sizeof(int) * inst->nnodes);
    double ttl = generate_random_path(inst, tpath);
    genetic_individual(&gen->parents->individual[i], inst->nnodes, tpath, ttl);
  }

  // QUICK REFINEMENT
  DEBUG_COMMENT("heuristic::genetic_algorithm", "Refining population");
  refine_pop(inst, gen->parents, 1);
  for (int i = 0; i < gen->population_size; i++) {
    add_solution(gen->parents->grasp_individuals, i,
                 gen->parents->individual[i].fitness);
  }

#ifndef PRODUCTION
  for (int i = 0; i < gen->grasp_n_probabilities; i++) {
    DEBUG_COMMENT("heuristic::genetic_algorithm", "Grasp %d - %d: %f", i,
                  gen->parents->grasp_individuals->solutions[i].solution,
                  gen->parents->grasp_individuals->solutions[i].value);
  }
#endif /* ifndef PRODUCTION */

  // START GENERATIONS --------------------------------------------------------
  if (gen->children) {
    gen->children = NULL;
    // genetic_destroy_population(gen->children);
  }
  double last_best = INFINITY;

  for (int iter = 0; iter < gen->ngenerations; iter++) {
    DEBUG_COMMENT("heuristic::genetic_algorithm", "Starting generation %d",
                  iter);
    if (gen->children) {
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
    gen->children = NULL;
    printf("finished generation %d\n", iter);
    DEBUG_COMMENT("heuristic::genetic_algorithm", "Selecting best solution");
    printf("best fitness %lf\n",
           gen->parents->grasp_individuals->solutions[0].value);
    DEBUG_COMMENT("heuristic::genetic_algorithm", "Finished generation %d",
                  iter);
    if (fabs(gen->parents->grasp_individuals->solutions[0].value - last_best) <
        1e-3)
      break;
    last_best = gen->parents->grasp_individuals->solutions[0].value;
  }

  DEBUG_COMMENT("heuristic::genetic_algorithm", "Selecting best solution");

  genetic_destroy_population(gen->parents);
  genetic_destroy_population(gen->children);
  return SUCCESS;
}
