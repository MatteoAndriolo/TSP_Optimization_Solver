#include "../include/greedy.h"

#include "../include/logger.h"
#include "../include/refinement.h"
#include "../include/utils.h"

void nearest_neighboor(const double *distance_matrix, int *path, int nnodes,
                       double *tour_length) {
  int current_node;
  int best_remaining = -1;
  DEBUG_COMMENT("greedy::nearest_neighboor", "start nearest neighboor");
  for (int j = 1; j < nnodes; j++) {
    current_node = path[j - 1];
    double min_distance = INFINITY;
    double dist = -1;
    for (int k = j; k < nnodes; k++) {
      dist = distance_matrix[current_node * nnodes + path[k]];
      if (dist < min_distance) {
        min_distance = dist;
        best_remaining = k;
      }
    }
    *tour_length += min_distance;
    DEBUG_COMMENT("greedy::model_nearest_neighboor",
                  "tour length, starting from %d, with %d nodes = %lf", path[0],
                  j, *tour_length);
    swap(path, j, best_remaining);
  }
  // complete the tour
  (*tour_length) += distance_matrix[path[0] * nnodes + path[nnodes - 1]];

  int path_is_ok = assert_path(path, distance_matrix, nnodes, *tour_length);
  if (!path_is_ok) {
    ERROR_COMMENT("greedy::model_nearest_neighboor", "path is not ok");
    *tour_length = -1;
  }
}

void nearest_neighboor_grasp(const double *distance_matrix, int *path,
                             const int nnodes, double *tour_length,
                             const double *probabilities, const int n_prob) {
  DEBUG_COMMENT("greedy::nng", "start nearest neighboor");

  int rankings_index[n_prob];
  double rankings_value[n_prob];

  // Nearest Neighboor -------------------------------------------
  int current_node;
  for (int j = 1; j < nnodes; j++) {
    // initialize rankings
    for (int i = 0; i < n_prob; i++) {
      rankings_index[i] =
          -1;  // first component is the index of the element plus 1
      rankings_value[i] =
          INFINITY;  // second component is a random value between 0 and 1
    }
    // find the best #n_prob neighboors
    current_node = path[j - 1];
    double min_distance = INFINITY;
    double dist = -1;
    for (int k = j; k < nnodes; k++) {
      dist = distance_matrix[current_node * nnodes + path[k]];
      if (dist < rankings_value[n_prob - 1])
        replace_if_better(rankings_index, rankings_value, n_prob, k, dist);
    }

    // pick one of the bests
    int index = -1;
    double r = rand() / RAND_MAX;  // generate a random number between 1-100
    DEBUG_COMMENT("greedy::nng", "probabilities %lf %lf %lf| r: %lf ",
                  probabilities[0], probabilities[1], probabilities[2], r);
    for (int i = 0; i < n_prob; i++) {
      if (r <= probabilities[i]) {
        index = i;
        break;
      }
    }
    DEBUG_COMMENT("greedy::nng", "%lf %lf %lf %lf -> %lf", r, rankings_value[0],
                  rankings_value[1], rankings_value[2], rankings_value[index]);
    min_distance = rankings_value[index];
    swap(path, j, rankings_index[index]);

    (*tour_length) += min_distance;
    DEBUG_COMMENT("greedy::nng",
                  "tour length, starting from %d, with %d nodes = %lf", path[0],
                  j, *tour_length);
  }
  // complete the tour
  (*tour_length) += distance_matrix[path[0] * nnodes + path[nnodes - 1]];
#ifndef PRODUCTION
  log_path(path, nnodes);
#endif
}

void extra_mileage(const double *distance_matrix, int *path, int nnodes,
                   double *tour_length) {
  //--------------- FIND DIAMETER -------------------------------------------
  double max_distance = 0;
  int max_index = -1;

  for (int i = 1; i < nnodes; i++) {
    if (distance_matrix[path[0] * nnodes + i] > max_distance &&
        distance_matrix[path[0] * nnodes + i] != INFINITY) {
      max_distance = distance_matrix[path[0] * nnodes + i];
      max_index = i;
    }
  }
  DEBUG_COMMENT("greedy::Extra_mileage", "max distance = %lf, at index = %d",
                max_distance, max_index);
  swap(path, 1, max_index);

  //--------------- START SEARCH -------------------------------------------
  *tour_length = 2 * max_distance;
  DEBUG_COMMENT("greedy::Extra_mileage", "initial tour length = %lf",
                tour_length);
  int node_3[3] = {-1, -1, -1};
  double min = INFINITY;

  for (int i = 2; i < nnodes; i++)  // starting with two path "visited"
  {
    min = INFINITY;
    DEBUG_COMMENT("greedy::Extra_mileage", "start iteration %d", i);
    double new_triangular_sum;
    int is_close_edge = 0;
    // Check from all the path to the following one

    for (int j = 0; j < i; j++) {
      for (int k = i; k < nnodes; k++) {
        new_triangular_sum = distance_matrix[path[j] * nnodes + path[k]] +
                             distance_matrix[path[j + 1] * nnodes + path[k]];
        if (new_triangular_sum < min) {
          min = new_triangular_sum;
          node_3[0] = j;
          node_3[1] = j + 1;
          node_3[2] = k;
        }
      }
    }

    for (int k = i; k < nnodes; k++) {
      new_triangular_sum = distance_matrix[path[0] * nnodes + path[k]] +
                           distance_matrix[path[i - 1] * nnodes + path[k]];
      if (new_triangular_sum < min) {
        min = new_triangular_sum;
        node_3[0] = 0;
        node_3[1] = i - 1;
        node_3[2] = k;
        is_close_edge = 1;
      }
    }

    // FOUND BEST NODE
    DEBUG_COMMENT("greedy::Extra_mileage",
                  "best value-->%f, best_triangle indices -->{%d,%d,%d}", min,
                  path[node_3[0]], path[node_3[1]], path[node_3[2]]);
    double new_cost =
        min - distance_matrix[path[node_3[0]] * nnodes + path[node_3[1]]];
    *tour_length += new_cost;
    DEBUG_COMMENT("greedy::Extra_mileage", "new cost-->%lf=%lf+%lf-%lf",
                  new_cost,
                  distance_matrix[path[node_3[0]] * nnodes + path[node_3[2]]],
                  distance_matrix[path[node_3[1]] * nnodes + path[node_3[2]]],
                  distance_matrix[path[node_3[0]] * nnodes + path[node_3[1]]]);
    DEBUG_COMMENT("greedy::Extra_mileage", "tour length-->%lf", tour_length);
    // SWAP - ADJUST PATH ----------------------------------------------------
    // Save the value at position j in a temporary variable
    if (is_close_edge) {
      int tmp = path[node_3[2]];  // save node in index best
      for (int k = node_3[2]; k > node_3[1]; k--) {
        path[k] = path[k - 1];
      }
      path[node_3[1] + 1] = tmp;
    } else {
      int tmp = path[node_3[2]];
      for (int k = node_3[2]; k >= node_3[1]; k--) {
        path[k] = path[k - 1];  // Shift all elements to the right from j to i+1
      }
      path[node_3[1]] = tmp;
    }
    DEBUG_COMMENT("greedy::Extra_mileage", "path shifted");
  }

  // Put the saved value from position j into position i
  if (assert_path(path, distance_matrix, nnodes, *tour_length))
    OUTPUT_COMMENT("greedy::extra_mileage", "found best tour lenght %lf",
                   tour_length);
}