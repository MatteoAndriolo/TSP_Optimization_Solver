#include "../include/utils.h"

void swap(int *arr, int i, int j) {
  int temp = arr[i];
  arr[i] = arr[j];
  arr[j] = temp;
}

void swap_and_shift(int *arr, int i, int j, int n) {
  int temp = arr[j];  // Save the value at position j in a temporary variable
  for (int k = j; k > i; k--) {
    arr[k] = arr[k - 1];  // Shift all elements to the right from j to i+1
  }
  arr[i] = temp;  // Put the saved value from position j into position i
}

void replace_if_better(int *r_index, double *r_value, int n_ranks,
                       int new_index, double new_value) {
  int i = 0;
  while (i < n_ranks && new_value > r_value[i]) i++;
  if (i < n_ranks) {
    for (int j = n_ranks - 1; j > i; j--) {
      r_value[j] = r_value[j - 1];
      r_index[j] = r_index[j - 1];
    }
    r_value[i] = new_value;
    r_index[i] = new_index;
  }
}

inline double distance_euclidean(double x1, double y1, double x2, double y2) {
  double dx = x1 - x2;
  double dy = y1 - y2;
  return sqrt(dx * dx + dy * dy);
}

// inline double distance_euclidean_square(double x1, double y1, double x2,
//                                         double y2) {
//   double dx = x1 - x2;
//   double dy = y1 - y2;
//   return dx * dx + dy * dy;
// }

void generate_distance_matrix(double **matrix, const int nnodes,
                              const double *x, const double *y, int r) {
  DEBUG_COMMENT("greedy::generate_distance_matrix",
                "Generating distance matrix");
  for (int i = 0; i < nnodes; i++) {
    for (int j = 0; j < nnodes; j++) {
      (*matrix)[i * nnodes + j] =
          round(distance_euclidean(x[i], y[i], x[j], y[j]));
    }
  }
  for (int i = 0; i < nnodes; i++) {
    (*matrix)[i * nnodes + i] = INFINITY;
  }
  DEBUG_COMMENT("greedy::generate_distance_*matrix",
                "Finised generating distance *matrixrix");
}

void set_starting_node(int *path, int starting_node, int num_nodes) {
  for (int i = 0; i < num_nodes; i++) {
    if (path[i] == starting_node) {
      swap(path, i, 0);
      return;
    }
  }
  ERROR_COMMENT("set_starting_node", "Starting node not found");
}

// CHECKS ----------------------------------------------------------------------
double get_tour_length(const int *path, const int nnodes,
                       const double *distance_matrix) {
  double tour_length = distance_matrix[path[0] * nnodes + path[nnodes - 1]];
  for (int i = 0; i < nnodes - 1; i++) {
    tour_length += distance_matrix[path[i] * nnodes + path[i + 1]];
  }
  return tour_length;
}

int feasiblePath(const int *path, const int nnodes) {
  // all nodes used
  int check_nnodes = (nnodes * (nnodes - 1)) / 2;
  for (int i = 0; i < nnodes; check_nnodes -= path[i++])
    ;

  return check_nnodes == 0;
}

int assert_path(const int *path, const double *distance_matrix,
                const int nnodes, const double tour_length) {
  // all nodes used
  int check_nnodes = (nnodes * (nnodes - 1)) / 2;
  for (int i = 0; i < nnodes; check_nnodes -= path[i++])
    ;

  // tour length correct
  double check_tour_length = 0;
  // distance_matrix[path[0] * nnodes + path[nnodes - 1]];
  for (int i = 0; i < nnodes - 1; i++) {
    check_tour_length += distance_matrix[path[i] * nnodes + path[i + 1]];
  }
  check_tour_length += distance_matrix[path[0] * nnodes + path[nnodes - 1]];

  if (check_nnodes != 0 || check_tour_length != tour_length) {
    ERROR_COMMENT("assert_path", "Path not valid");
    printf("check_nnodes: %d\n", check_nnodes);
    printf("check_tour_length: %lf\n", check_tour_length);

    exit(-1);
  }
  return 1;
}

void generate_random_starting_nodes(int *starting_nodes, int num_nodes,
                                    int num_instances, int seed) {
  srand(seed);
  int found = 0;
  while (found < num_instances) {
    int r = rand() % num_nodes;
    int i = 0;
    while (i < found && starting_nodes[i] != r) i++;
    // generate num_instances random number without repetition in range (0,
    // num_nodes)
    if (i == found) {
      starting_nodes[found] = r;
      found++;
    }
  }
}


void swap_array_piece(int *arr, int start1, int end1, int start2, int end2) {
  int temp[end1 - start1 +
           1];  // Create a temporary array to hold the first piece
  memcpy(temp, &arr[start1],
         sizeof(int) *
             (end1 - start1 + 1));  // Copy the first piece to the temp array
  memcpy(&arr[start1], &arr[start2],
         sizeof(int) *
             (end1 - start1 + 1));  // Copy the second piece to the first piece
  memcpy(&arr[start2], temp,
         sizeof(int) *
             (end1 - start1 +
              1));  // Copy the temp array (first piece) to the second piece
}

void generate_path(int *path, const int starting_node, const int num_nodes, const bool shuffle) {
    for (int i = 0; i < num_nodes; i++) {
        path[i] = i;
    }
    path[starting_node] = 0;
    path[0] = starting_node;

    if (shuffle) {
        int j, tmp;
        for (int i = 0; i < num_nodes; i++) {
            j = rand() % num_nodes;
            tmp = path[i];
            path[i] = path[j];
            path[j] = tmp;
        }
    }
}


void two_opt_move(int *path, int n1, int n2, int nnodes) {
  // DEBUG_COMMENT("utils::two_opt_move", "Entering two_opt_move function %d
  // %d", n1, n2);
  int t = (n1 + 1) % nnodes;
  for (int z = 0; z < (int)(n2 - n1 + 1) / 2;
       z++)  // reverse order cells (n1+1,index_min)
  {
    double temp = path[z + t];
    path[z + t] = path[n2 - z];
    path[n2 - z] = temp;
  }
}


int randomBetween(int lowerBound, int upperBound) {
  int randomBetween = (rand() % (upperBound - lowerBound + 1)) + lowerBound;
  return randomBetween;
}

bool simpleCorrectness(int *path, int nnodes) {
  int *check = malloc(nnodes * sizeof(int));
  for (int i = 0; i < nnodes; i++) {
    check[i] = 0;
  }
  for (int i = 0; i < nnodes; i++) {
    check[path[i]]++;
  }
  for (int i = 0; i < nnodes; i++) {
    if (check[i] != 1) {
      ERROR_COMMENT("utils::simpleCorrectness", "Error in path: %d, index %d",
                    path[i], i);
      return false;
    }
  }
  return true;
}
