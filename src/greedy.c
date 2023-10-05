#include "../include/greedy.h"

#include "../include/errors.h"
#include "../include/grasp.h"
#include "../include/logger.h"
#include "../include/refinement.h"
#include "../include/utils.h"

int nearest_neighboor(Instance *inst) {
  INFO_COMMENT("nearest_neighboor", "start nearest neighboor");
  int current_node;
  GRASP_Framework *grasp = inst->grasp;
  double dist;
  /*
   * 1. Find closest node to the current node
   * 2. Make it the next node in the inst->path
   */
  for (int j = 1; j < inst->nnodes; j++) {
    current_node = inst->path[j - 1];
    for (int k = j; k < inst->nnodes; k++) {
      dist = getDistanceNodes(inst, current_node, inst->path[k]);
      add_solution(grasp, k, dist);
    }

    swapPathPoints(inst, j, get_solution(grasp));
    reset_solutions(grasp);
  }
  calculateTourLength(inst);

  ASSERTINST(inst);
  return SUCCESS;
}

int extra_mileage(Instance *inst) {
  INFO_COMMENT("greedy::extra_mileage", "start extra mileage");
  //--------------- FIND DIAMETER -------------------------------------------
  // TODO find diameter | farthest with lowest mean distance from other nodes
  double max_distance = 0;
  double tdist;
  int max_index = -1;

  for (int i = 1; i < inst->nnodes; i++) {
    tdist = getDistancePos(inst, 0, i);
    if (tdist > max_distance && tdist != INFINITY) {
      max_distance = tdist;
      max_index = i;
    }
  }
  tdist = getDistancePos(inst, 0, inst->nnodes - 1);
  if (tdist > max_distance && tdist != INFINITY) {
    max_distance = tdist;
    max_index = inst->nnodes - 1;
  }

  swapPathPoints(inst, 1, max_index);
  setTourLenght(inst, 2 * max_distance);
  // write nodes

  //--------------- START SEARCH -------------------------------------------
  int node_3[3] = {-1, -1, -1};
  double min_nts = INFINITY;

  // add all the remaining nodes -> starts from current number of nodes ->
  // 2(diameter) check for all remaining node all the best possible positions in
  // between the current nodes
  for (int i = 2; i < inst->nnodes; i++) {
    min_nts = INFINITY;
    double new_triangular_sum;
    bool is_close_edge = false;

    // Check from all the inst->path to the following one
    for (int j = 0; j < i - 1; j++) {
      for (int k = i; k < inst->nnodes; k++) {
        new_triangular_sum =
            getDistancePos(inst, j, k) + getDistancePos(inst, j + 1, k);
        if (new_triangular_sum < min_nts) {
          min_nts = new_triangular_sum;
          node_3[0] = j;
          node_3[1] = j + 1;
          node_3[2] = k;
        }
        CHECKTIME(inst, false);
      }
    }

    for (int k = i; k < inst->nnodes; k++) {
      new_triangular_sum =
          getDistancePos(inst, 0, k) + getDistancePos(inst, i - 1, k);
      if (new_triangular_sum < min_nts) {
        min_nts = new_triangular_sum;
        node_3[0] = 0;
        node_3[1] = i - 1;
        node_3[2] = k;
        is_close_edge = true;
      }
      CHECKTIME(inst, false);
    }
    double new_cost = min_nts - getDistancePos(inst, node_3[0], node_3[1]);

    addToTourLenght(inst, new_cost);

    // SWAP - ADJUST PATH ----------------------------------------------------
    // Save the value at position j in a temporary variable
    if (is_close_edge) {
      int tmp = inst->path[node_3[2]];  // save node in index best
      for (int k = node_3[2]; k > node_3[1]; k--) {
        inst->path[k] = inst->path[k - 1];
      }
      inst->path[node_3[1] + 1] = tmp;
    } else {
      int tmp = inst->path[node_3[2]];
      for (int k = node_3[2]; k >= node_3[1]; k--) {
        inst->path[k] =
            inst->path[k - 1];  // Shift all elements to the right from j to i+1
      }
      inst->path[node_3[1]] = tmp;
    }

    CHECKTIME(inst, false);
  }

  ASSERTINST(inst);
  return SUCCESS;
}
