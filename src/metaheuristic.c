#include "../include/metaheuristic.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../include/errors.h"
#include "../include/genetic.h"
#include "../include/refinement.h"
#include "../include/utils.h"

double energy_probabilities(double cost_current, double cost_new, double T,
                            double coefficient) {
  // *coefficient;
  if (cost_new < cost_current) return 1;
  double delta = fabs(cost_current - cost_new) /
                 cost_current;  // (cost_current + cost_new);
  INFO_COMMENT("metaheuristic.c:energy_probabilities",
               "delta: %lf, T: %lf, exp(-delta / T): %lf", delta, T,
               exp(-delta / T));
  return exp(-delta / T);
  // return exp(-coefficient / T);
}

void backupInstState(const Instance *inst, int *path, double *tour_length) {
  memcpy(path, inst->path, inst->nnodes * sizeof(int));
  *tour_length = inst->tour_length;
}

void restoreInstState(Instance *inst, const int *path, const int tour_length) {
  memcpy(inst->path, path, inst->nnodes * sizeof(int));
  inst->tour_length = tour_length;
}

int simulated_annealling(Instance *inst, double k_max) {
  INFO_COMMENT("metaheuristic.c:simulate_anealling",
               "Starting simulated annealing metaheuristic");
  double T, rand_val, energy;

  two_opt_noupdate(inst, INFINITY);
  // backup the current state
  int *path = (int *)malloc(inst->nnodes * sizeof(int));
  double tour_length;
  backupInstState(inst, path, &tour_length);
  INFO_COMMENT("metaheuristic.c:simulate_anealling", "Starting sa %lf",
               tour_length);
  // Main loop iterating through k_max iterations
  for (int k = 0; k < k_max; k++) {
    T = 1 - ((double)(k + 1) / k_max);

    // Modify and optimize current solution
    RUN(kick_function(inst, k_max - k));
    // INSTANCE_calculateTourLength(inst);
    // INSTANCE_pathCheckpoint(inst);
    RUN(two_opt_noupdate(inst, INFINITY));

    // Calculate acceptance probability and generate a rand_val number
    // printf("tour_length: %lf, inst->tour_length: %lf, T: %f\n", tour_length,
    //       inst->tour_length, T);
    energy = energy_probabilities(tour_length, inst->tour_length, T, 0.8);
    rand_val = randomBetween_d(0, 1);
    // INSTANCE_pathCheckpoint(inst);
    //  Debugging output
    DEBUG_COMMENT("metaheuristic.c:simulate_anealling",
                  "k: %d, T: %f, energy: %f, rand_val: %f", k, T, energy,
                  rand_val);
    // printf("k: %d, T: %f, energy: %f, rand_val: %f\n", k, T, energy,
    // rand_val);
    if (energy < rand_val) {  // rejected !
      // printf("rejected\n");
      INFO_COMMENT("metaheuristic.c:simulate_anealling", "rejected %d",
                   inst->curr_iter);
      restoreInstState(inst, path, tour_length);
    } else {  // accepted
      backupInstState(inst, path, &tour_length);
    }
    INSTANCE_pathCheckpoint(inst);
    CHECKTIME(inst, false);
  }

  free(path);
  INFO_COMMENT("metaheuristic.c:simulate_anealling",
               "Simulated annealing metaheuristic finished");
  return SUCCESS;
}

// ----------------------------------------------------------------------------
// VNS
// ----------------------------------------------------------------------------

ErrorCode vns_k(Instance *inst, int maxJ, int minJ, int iterations) {
  INFO_COMMENT("metaheuristic.c:simulate_anealling",
               "Starting simulated annealing metaheuristic");

  // two_opt_noupdate(inst, INFINITY);
  //  backup the current state
  int *path = (int *)malloc(inst->nnodes * sizeof(int));
  double tour_length;
  backupInstState(inst, path, &tour_length);
  RUN(INSTANCE_pathCheckpoint(inst));

  int jump = minJ;
  // Main loop iterating through k_max iterations
  while (jump <= maxJ) {
    // Modify and optimize current solution
    RUN(kick_function(inst, jump));
    RUN(two_opt_noupdate(inst, INFINITY));
    INSTANCE_pathCheckpoint(inst);

    if (tour_length < inst->tour_length) {
      restoreInstState(inst, path, tour_length);
      jump++;
    } else {
      backupInstState(inst, path, &tour_length);
      jump = minJ;
    }
    RUN(INSTANCE_pathCheckpoint(inst));
    //  Debugging output
    CHECKTIME(inst, false);
  }

  free(path);
  INFO_COMMENT("metaheuristic.c:vns_k",
               "Simulated annealing metaheuristic finished");
  return SUCCESS;
}

// ErrorCode vns_k(Instance *inst, int maxJ, int minJ, int iterations) {
//   int c = 0, k = -1, totiter = 0;
//   int niterations = inst->nnodes * 2;
//   // double ttl = -1, ttl2 = -1;
//
//   if (maxJ < 0 || minJ < 0 || maxJ > minJ) {
//     ERROR_COMMENT("metaheuristic.c:vnp_k",
//                   "invalid start and end values for vns_k");
//   }
//
//   while (totiter++ < niterations) {
//     c++;
//     k = maxJ - c;
//     INFO_COMMENT("metaheuristic.c:vnp_k",
//                  "starting the heuristics loop for vnp_k, iteration %d", c);
//     // ttl = inst->best_tourlength;
//
//     printf("k: %d\n", k);
//     RUN(kick_function(inst, k));
//     // RUN(two_opt_tabu(inst, 10, initializeTabuList(20, 7)));
//     RUN(two_opt_noupdate(inst, INFINITY));
//     // ttl2 = inst->best_tourlength;
//     RUN(INSTANCE_pathCheckpoint(inst));
//     CHECKTIME(inst, false);
//
//     // c = linear_parameters(&adp, (ttl - ttl2) / ttl);
//     //  if (fabs(ttl2 - ttl) <
//     //      1e-3) { // ++nWorst > iterations / 10) {   // if found many worst
//     //    if (decreasing) {
//     //      c = (int)(c / 2); // increase jump
//     //    } else {
//     //      c = niterations - (int)(c / 4); // make jump smaller
//     //    }
//     //    decreasing = !decreasing;
//     //  } else if (restart && fabs(ttl2 - ttl) < 1e-3 * ttl) {
//     //    break;
//     //  } else {
//     //    restart = 0;
//     //    nWorst = 0;
//     //  }
//     // INFO_COMMENT("metaheuristic.c:vnp_k", "vns iter %d, k=%d, ttl= %lf",
//     c,
//     // k,
//     //             ttl2);
//     INFO_COMMENT("metaheuristic.c:vns_k", "tot iter %d", totiter);
//   }
//   INFO_COMMENT("metaheuristic.c:vnp_k",
//                "finished the huristics loop for vnp_k, tot iter %d",
//                totiter);
//   DEBUG_COMMENT("metaheuristic.c:vnp_k", "best tourlength: %lf",
//                 inst->best_tourlength);
//   return SUCCESS;
// }

int _kick_function(Instance *inst, int k, TabuList *tabu) {
  for (int i = 0; i < k; i++) {
    int found = 0;
    int index_0 = 0, index_1 = 0, index_2 = 0, index_3 = 0, index_4 = 0;
    while (found != 4) {
      found = 0;
      index_0 = 1;
      found++;
      index_1 = randomBetween(index_0 + 1, inst->nnodes);
      if (inst->nnodes - index_1 > 6) {
        index_2 = randomBetween(index_1 + 2, inst->nnodes);
        found++;
      }
      if (inst->nnodes - index_2 > 4) {
        index_3 = randomBetween(index_2 + 2, inst->nnodes);
        found++;
      }
      if (inst->nnodes - index_3 > 2) {
        index_4 = randomBetween(index_3 + 2, inst->nnodes);
        found++;
      }
      if (found == 4) {
        index_0--;
        index_1--;
        index_2--;
        index_3--;
        index_4--;
        // DEBUG_COMMENT("metaheuristic.c:kiint ck_function", "found 5
        // indexes{%d, %d, %d, %d, %d}", index_0, index_1, index_2, index_3,
        // index_4);
      }
      CHECKTIME(inst, false);
    }

    if (tabu) {
      addTabu(tabu, index_1);
      addTabu(tabu, index_3);
    }
    //---------------------------------------------------------------------------------------
    int *final_path = malloc(inst->nnodes * sizeof(int));
    int count = 0;
    for (int i = 0; i < index_1; i++) final_path[count++] = inst->path[i];
    for (int i = index_4; i < inst->nnodes; i++)
      final_path[count++] = inst->path[i];
    for (int i = index_4 - 1; i >= index_3; i--)
      final_path[count++] = inst->path[i];
    for (int i = index_2; i < index_3; i++) final_path[count++] = inst->path[i];
    for (int i = index_2 - 1; i >= index_1; i--)
      final_path[count++] = inst->path[i];
    free(inst->path);
    inst->path = final_path;
  }

  return SUCCESS;
}
ErrorCode kick_function_tabu(Instance *inst, int k, TabuList *tabu) {
  return _kick_function(inst, k, tabu);
}

ErrorCode kick_function(Instance *inst, int k) {
  return _kick_function(inst, k, NULL);
}

// ----------------------------------------------------------------------------
// Genetic
// ----------------------------------------------------------------------------

void refine_pop(Instance *inst, GENETIC_POPULATION *population,
                time_t duration) {
  for (int i = 0; i < population->population_size; i++) {
    Instance *I =
        (Instance *)temp_instance(inst, population->individual[i].path);
    I->tend = time(NULL) + duration;
    two_opt_noupdate(I, inst->curr_iter * inst->curr_iter);

    population->individual[i].fitness = I->tour_length;
    GRASP_addSolution(population->grasp_individuals, i, I->tour_length);
    // printf("%d %lf\n", inst->curr_iter, population->individual[i].fitness);
    FREE(I);
  }
}

int _insert_node(GENETIC_INDIVIDUAL *src, GENETIC_INDIVIDUAL *dest,
                 bool *inserted, int *pos, int *iter, int *size) {
  int err = SUCCESS;
  if (inserted[src->path[*pos]]) {
    DEBUG_COMMENT("metaheuristic.c::crossover", "ERR: already inserted %d",
                  src->path[*pos]);
    err = ERROR;
  } else {
    dest->path[*size] = src->path[*pos];
    DEBUG_COMMENT("metaheuristic.c::crossover", "OK : inserted %d",
                  dest->path[*size]);
    (*size)++;
    inserted[src->path[*pos]] = true;
  }
  *pos = (*pos + 1) % dest->nnodes;
  (*iter)++;

  return err;
}

void crossover(Instance *inst, GENETIC_SETUP *gen) {
  DEBUG_COMMENT("metaheuristic.c::crossover", "ENTERING FUNCTION");
  DEBUG_COMMENT("metaheuristic.c::crossover", "Finding candidates");
  double prob_cross[] = {0.65, 0.35};

  for (int i = 0; i < gen->population_size; i++) {
    int ch = GRASP_getSolution(gen->parents->grasp_individuals);
    GENETIC_INDIVIDUAL *champion = &gen->parents->individual[ch];
    GENETIC_INDIVIDUAL *chosen;
    int rnd;
    DEBUG_COMMENT("metaheuristic.c::crossover",
                  "Finding candidates for champion %d -> %d", i, ch);
    do {
      rnd = rand() % gen->population_size;
    } while (!GRASP_isInGrasp(gen->parents->grasp_individuals, rnd));
    chosen = &gen->parents->individual[rnd];

    DEBUG_COMMENT("metaheuristic.c::crossover", "Chosen %d", rnd);

    int s1 = 0, s2 = 0;
    DEBUG_COMMENT("metaheuristic.c::crossover", "Finding starting nodes %d",
                  inst->starting_node);
    for (; champion->path[s1] != inst->starting_node && s1 < inst->nnodes; s1++)
      ;
    for (; chosen->path[s2] != inst->starting_node && s2 < inst->nnodes; s2++)
      ;
    DEBUG_COMMENT("metaheuristic.c::crossover", "Starting nodes found %d %d",
                  s1, s2);

    DEBUG_COMMENT("metaheuristic.c::crossover", "Starting crossover");
    int size = 0;
    bool *inserted = malloc(inst->nnodes * sizeof(bool));
    for (int i = 0; i < inst->nnodes; i++) {
      inserted[i] = false;
    }
    if (!inserted) {
      ERROR_COMMENT("metaheuristic.c::crossover", "CALLOC ERROR");
      exit(ERROR_ALLOCATION);
    }

    GENETIC_INDIVIDUAL child;
    child.path = malloc(sizeof(int) * inst->nnodes);
    if (!child.path) {
      ERROR_COMMENT("metaheuristic.c::crossover", "CALLOC ERROR");
      free(inserted);
      exit(ERROR_ALLOCATION);
    }
    child.fitness = 0;
    child.nnodes = inst->nnodes;

    int i1 = 0, i2 = 0;
    int p1 = s1 + i1;
    int p2 = s2 + i2;
    _insert_node(champion, &child, inserted, &p1, &i1, &size);
    DEBUG_COMMENT("metaheuristic.c::crossover", "inserted %d",
                  child.path[size]);
    i2++;
    p2 = (s2 + i2) % child.nnodes;

    bool isChampion = true;
    while (i1 < inst->nnodes || i2 < inst->nnodes) {
      DEBUG_COMMENT("metaheuristic.c::crossover", "size %d", size);
      if (i1 >= inst->nnodes || i2 >= inst->nnodes) {
        if (i1 < inst->nnodes) {
          isChampion = true;
        } else if (i2 < inst->nnodes) {
          isChampion = false;
        } else {
          ERROR_COMMENT("metaheuristic.c::crossover",
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
        DEBUG_COMMENT("metaheuristic.c::crossover", "Champion");
        while (_insert_node(champion, &child, inserted, &p1, &i1, &size) !=
                   SUCCESS &&
               i1 < inst->nnodes)
          ;
      }

      else {
        DEBUG_COMMENT("metaheuristic.c::crossover", "Chosen");
        while (_insert_node(chosen, &child, inserted, &p2, &i2, &size) !=
                   SUCCESS &&
               i2 < inst->nnodes)
          ;
      }
      DEBUG_COMMENT("metaheuristic.c::crossover", "size %d", size);
      DEBUG_COMMENT("metaheuristic.c::crossover", "i1 %d,i2 %d, p1 %d, p2 %d",
                    i1, i2, p1, p2);
    }

    DEBUG_COMMENT("metaheuristic.c::crossover", "Exit FINAL while");

    // ------------------ DEBUG ------------------
    if (size != inst->nnodes) {
      for (int l = 0; l < inst->nnodes; l++)
        if (!inserted)
          DEBUG_COMMENT("metaheuristic.c::crossover", "missing node[%d]=%d", l,
                        chosen->path[l]);
      ErrorCode err =
          assert_path(child.path, inst->distance_matrix, inst->nnodes, -1);
      if (err == ERROR_INVALID_PATH)
        for (int l = 0; l < inst->nnodes; l++)
          DEBUG_COMMENT("metaheuristic.c::crossover", "errAss path[%d]=%d", l,
                        child.path[l]);
      ERROR_COMMENT("metaheuristic.c::crossover", "size is not nnodes");
      exit(-1);
    }
    if (i1 < inst->nnodes) {
      ERROR_COMMENT("metaheuristic.c::crossover", "i1 is not nnodes");
      exit(-1);
    }
    if (i2 < inst->nnodes) {
      ERROR_COMMENT("metaheuristic.c::crossover", "i2 is not nnodes");
      exit(-1);
    }
    // ------------------ ENDDEBUG ------------------

    child.fitness = calculateTourLenghtPath(inst, child.path);

    gen->children->individual[i] = child;
    free(inserted);
  }

  DEBUG_COMMENT("metaheuristic.c::crossover", "EXITING FUNCTION");
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
ErrorCode genetic_algorithm(Instance *inst) {
  DEBUG_COMMENT("metaheuristic.c::genetic_algorithm",
                "Starting genetic algorithm");

  // BASIC SETUP
  int n_prob = 6;
  double *temp_prob = malloc(sizeof(double) * n_prob);
  for (int i = 1; i <= n_prob; i++) {
    if (i == 1)
      temp_prob[i - 1] = 0.5;
    else
      temp_prob[i - 1] = (double)(1 / pow(2, i)) + temp_prob[i - 2];
    DEBUG_COMMENT("metaheuristic.c::genetic_algorithm", "Grasp %d: %f", i,
                  temp_prob[i - 1]);
  }
  GENETIC_SETUP *gen = inst->genetic_setup;
  gen = (GENETIC_SETUP *)malloc(sizeof(GENETIC_SETUP));

  genetic_setup(gen, inst->ngenerations, inst->population_size,
                (int)(inst->nnodes / 10), n_prob, temp_prob);

  // Set Best Individual
  gen->best_individual = malloc(sizeof(GENETIC_INDIVIDUAL));
  GENETIC_initIndividual(gen->best_individual, inst->nnodes, NULL, INFINITY);
  DEBUG_COMMENT("metaheuristic.c::genetic_algorithm", "Best individual set");
  temp_prob = NULL;

  // INIT FIRST GENERATION
  gen->parents = malloc(sizeof(GENETIC_POPULATION));
  GENETIC_initPopulation(gen->parents, gen);
  DEBUG_COMMENT("metaheuristic.c::genetic_algorithm",
                "Generating first population");
  for (int i = 0; i < gen->population_size; i++) {
    int *tpath = malloc(sizeof(int) * inst->nnodes);
    double ttl = generate_random_path(inst, tpath);
    GENETIC_initIndividual(&gen->parents->individual[i], inst->nnodes, tpath,
                           ttl);
  }

  // QUICK REFINEMENT
  DEBUG_COMMENT("metaheuristic.c::genetic_algorithm", "Refining population");
  refine_pop(inst, gen->parents, 1);
  for (int i = 0; i < gen->population_size; i++) {
    GRASP_addSolution(gen->parents->grasp_individuals, i,
                      gen->parents->individual[i].fitness);
  }
#ifndef PRODUCTION
  for (int i = 0; i < gen->grasp_n_probabilities; i++) {
    DEBUG_COMMENT("metaheuristic.c::genetic_algorithm", "Grasp %d - %d: %f", i,
                  gen->parents->grasp_individuals->solutions[i].solution,
                  gen->parents->grasp_individuals->solutions[i].value);
  }
#endif /* ifndef PRODUCTION */

  // START GENERATIONS
  // --------------------------------------------------------
  if (gen->children) {
    gen->children = NULL;
    // genetic_destroy_population(gen->children);
  }
  // double last_best = INFINITY;

  for (int iter = 0; iter < gen->ngenerations; iter++) {
    DEBUG_COMMENT("metaheuristic.c::genetic_algorithm",
                  "Starting generation %d", iter);
    if (gen->children) {
      ERROR_COMMENT("metaheuristic.c::genetic_algorithm",
                    "children is not NULL");
      exit(-1);
    }
    gen->children = malloc(sizeof(GENETIC_POPULATION));
    GENETIC_initPopulation(gen->children, gen);

    // GENERATE CHILDRENS
    DEBUG_COMMENT("metaheuristic.c::genetic_algorithm", "Crossover");
    crossover(inst, gen);

    // refine children
    DEBUG_COMMENT("metaheuristic.c::genetic_algorithm", "Refining children");
    refine_pop(inst, gen->children, 3);

    INSTANCE_setTourLenght(inst, gen->children->individual[0].fitness);
    // printf("%d, best fitness %lf\n", iter,
    // gen->children->individual[0].fitness);
    INSTANCE_storeCost(inst);
    DEBUG_COMMENT("metaheuristic.c::genetic_algorithm",
                  "Selecting best children");
    genetic_destroy_population(gen->parents);

    gen->parents = gen->children;
    gen->children = NULL;
    // printf("finished generation %d\n", iter);
    DEBUG_COMMENT("metaheuristic.c::genetic_algorithm",
                  "Selecting best solution");
    // printf("best fitness %lf\n",
    //        gen->parents->grasp_individuals->solutions[0].value);
    DEBUG_COMMENT("metaheuristic.c::genetic_algorithm",
                  "Finished generation %d", iter);
    // last_best = gen->parents->grasp_individuals->solutions[0].value;
    //  stop condition
    //  if (fabs(gen->parents->grasp_individuals->solutions[0].value -
    //  last_best)
    //       < 1e-3){
    //   break;
    //   }
  }

  DEBUG_COMMENT("metaheuristic.c::genetic_algorithm",
                "Selecting best solution");

  genetic_destroy_population(gen->parents);
  genetic_destroy_population(gen->children);
  return SUCCESS;
}
