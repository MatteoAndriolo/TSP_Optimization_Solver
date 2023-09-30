#include "../include/metaheuristic.h"

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

void simulate_anealling(Instance *inst, double k_max) {
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
    kick_function(inst, 4);
    two_opt(inst, INFINITY);

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
      pathCheckpoint(inst);
    }
  }

  free(path);
  INFO_COMMENT("metaheuristic.c:simulate_anealling",
               "Simulated annealing metaheuristic finished");
}
