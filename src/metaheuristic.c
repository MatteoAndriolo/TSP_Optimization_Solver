#include "../include/metaheuristic.h"

double randomBetween_d(double lowerBound, double upperBound) {
  double randomBetween = (double)rand() / RAND_MAX;
  randomBetween = randomBetween * (upperBound - lowerBound) + lowerBound;
  return randomBetween;
}

double energy_probabilities(double cost_current, double cost_new, double T,
                            double coefficient) {
  // double delta = (cost_new - cost_current) / (cost_current + cost_new) *
  // coefficient;
  if (cost_new < cost_current) return 1;
  return exp(-coefficient / T);
}

void simulate_anealling(const double *distance_matrix, int *path, int nnodes,
                        double *tour_length, double k_max, int duration) {
  INFO_COMMENT("metaheuristic.c:simulate_anealling",
               "Starting simulated annealing metaheuristic");
  double T;

  int *current_state = malloc(nnodes * sizeof(int));
  double current_state_cost = *tour_length;
  memcpy(current_state, path, nnodes * sizeof(int));
  double old_coefficient_cost = 0;
  int *new_state = malloc(nnodes * sizeof(int));
  double new_state_cost;

  for (int k = 0; k < k_max; k++) {
    T = (1 - ((double)k / k_max));
    old_coefficient_cost = current_state_cost;
    kick_function(distance_matrix, current_state, nnodes, &current_state_cost,
                  4);
    two_opt(distance_matrix, nnodes, current_state, &current_state_cost, INFINITY, duration);
    new_state_cost = current_state_cost;
    memcpy(new_state, current_state, nnodes * sizeof(int));
    double value =
        energy_probabilities(old_coefficient_cost, new_state_cost, T, 0.8);
    double random = randomBetween_d(0, 1);
    DEBUG_COMMENT("metaheuristic.c:simulate_anealling",
                  "k: %d, T: %f, value: %f, random: %f", k, T, value, random);
    if (value > random) {
      memcpy(current_state, new_state, nnodes * sizeof(int));
      current_state_cost = new_state_cost;
      CRITICAL_COMMENT("metaheuristic.c:simulate_anealling",
                       "New state accepted, cost: %f", current_state_cost);
    }
  }
  memcpy(path, current_state, nnodes * sizeof(int));
  *tour_length = current_state_cost;
  INFO_COMMENT("metaheuristic.c:simulate_anealling",
               "Simulated annealing metaheuristic finished");
}
