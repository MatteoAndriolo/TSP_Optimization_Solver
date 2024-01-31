#include "../include/metaheuristic.h"

#include <math.h>
#include <string.h>

#include "../include/refinement.h"
#include "../include/utils.h"

double energy_probabilities(double cost_current, double cost_new, double T,
                            double coefficient) {
  // double delta = (cost_new - cost_current) / (cost_current + cost_new) *
  // coefficient;
  if (cost_new < cost_current) return 1;
  return exp(-coefficient / T);
}

void backupInstState(const Instance *inst, int *path, int *tour_length) {
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

  // backup the current state
  int *path = (int *)malloc(inst->nnodes * sizeof(int));
  int tour_length;
  backupInstState(inst, path, &tour_length);

  // Main loop iterating through k_max iterations
  for (int k = 0; k < k_max; k++) {
    T = 1 - ((double)k / k_max);

    // Modify and optimize current solution
    RUN(kick_function(inst, 4));
    RUN(two_opt(inst, INFINITY));

    // Calculate acceptance probability and generate a rand_val number
    energy = energy_probabilities(tour_length, inst->tour_length, T, 0.8);
    rand_val = randomBetween_d(0, 1);

    // Debugging output
    DEBUG_COMMENT("metaheuristic.c:simulate_anealling",
                  "k: %d, T: %f, energy: %f, rand_val: %f", k, T, energy,
                  rand_val);

    if (rand_val < energy) {  // rejected !
      restoreInstState(inst, path, tour_length);
    } else {  // accepted
      backupInstState(inst, path, &tour_length);
      INSTANCE_pathCheckpoint(inst);
    }
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

int jumpDimension(int start, int end, int curriter, int maxiter) {
  int jump = start - (int)((start - end) * (double)curriter / (double)maxiter);
  jump = jump == 0 ? 1 : jump;
  if (jump < 0) {
    FATAL_COMMENT("metaheuristic.c:jumpDimension", "jump < 0");
  }
  INFO_COMMENT("metaheuristic.c:jumpDimension", "jump: %d", jump);
  return jump;
}

ErrorCode vns_k(Instance *inst, int start, int end, int iterations) {
  int c = 0, k = -1, totiter = 0;
  int niterations = inst->nnodes * 2;
  double ttl = -1, ttl2 = -1;

  if (start < 0 || end < 0 || start > end) {
    ERROR_COMMENT("metaheuristic.c:vnp_k",
                  "invalid start and end values for vns_k");
  }

  int nWorst = 0;
  int restart = 0;
  /**
   * While loopd for the vns
   *
   * Jump lenght calculated based on
   * - current iteration and it is used to calculate the coefficient for the
   *   linear function used to calculate the jump lenght
   *   - current iteration is changed when the number of iterations without
   *     improvement is reached
   *     - first it is reduced (smaller jumps)
   *     - then it is enlarged (larger jumps)
   *     - repeat until even when enlarged no improvements are made
   *
   *
   * Ends when:
   * - the number of iterations is reached
   *   OR
   *   the number of iterations without improvement is reached
   */
  while (++c < niterations) {
    totiter++;
    k = jumpDimension(start, end, c, niterations);
    INFO_COMMENT("metaheuristic.c:vnp_k",
                 "starting the heuristics loop for vnp_k, iteration %d", c);
    ttl = inst->best_tourlength;
    RUN(kick_function(inst, k));
    RUN(two_opt_tabu(inst, 100, initializeTabuList(20, 7)));
    ttl2 = inst->best_tourlength;
    INSTANCE_calculateTourLength(inst);
    INSTANCE_pathCheckpoint(inst);
    CHECKTIME(inst, false);
    if (ttl2 >= ttl && ++nWorst > iterations / 10) {
      if (restart == 0) {
        c = niterations - (int)(c / 4);
        restart = 1;
      } else if (restart == 1) {
        c = (int)(c / 2);
        restart = 2;
      } else if (restart == 2) {
        break;
      }
      nWorst = 0;
    } else if (restart && ttl2 - ttl < 0.005 * ttl) {
      break;
    } else {
      restart = 0;
    }
    INFO_COMMENT("metaheuristic.c:vnp_k", "vns iter %d, k=%d, ttl= %lf", c, k,
                 ttl2);
    INFO_COMMENT("metaheuristic.c:vns_k", "tot iter %d", totiter);
  }
  INFO_COMMENT("metaheuristic.c:vnp_k",
               "finished the huristics loop for vnp_k, tot iter %d", totiter);
  DEBUG_COMMENT("metaheuristic.c:vnp_k", "best tourlength: %lf",
                inst->best_tourlength);
  return SUCCESS;
}

int kick_function(Instance *inst, int k) {
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
      // DEBUG_COMMENT("metaheuristic.c:kick_function", "found 5 indexes{%d, %d,
      // %d, %d, %d}", index_0, index_1, index_2, index_3, index_4);
    }
    CHECKTIME(inst, false);
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
  return SUCCESS;
}
