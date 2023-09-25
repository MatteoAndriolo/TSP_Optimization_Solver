#include "../include/greedy.h"

#include "../include/errors.h"
#include "../include/logger.h"
#include "../include/refinement.h"
#include "../include/utils.h"


int nearest_neighboor(Instance *inst) {
    int current_node;
    double min_distance;
    int best_remaining = -1;

    double dist;
    /*
     * 1. Find closest node to the current node
     * 2. Make it the next node in the inst->path
     */
    for (int j = 1; j < inst->nnodes; j++) {
        current_node = inst->path[j - 1];
        min_distance = INFINITY;
        // find min
        for (int k = j; k < inst->nnodes; k++) {
            //dist = inst->distance_matrix[current_node * inst->nnodes + inst->path[k]];
            dist= getDistanceNodes(inst, current_node, inst->path[k]);
            if (dist < min_distance) {
                min_distance = dist;
                best_remaining = k;
            }
        }

        //addToTourLenght(inst, min_distance);
        swapPathPoints(inst, j, best_remaining);
    }
    // complete the tour
    //addToTourLenght(inst,getDistance(inst,inst->path[0], inst->path[inst->nnodes-1]));
    //inst->tour_length += inst->distance_matrix[inst->path[0] * inst->nnodes + inst->path[inst->nnodes - 1]];
    calculateTourLength(inst);

    //assert_path(inst->path, inst->distance_matrix, inst->nnodes, inst->tour_length);
    assertInst(inst);
    return SUCCESS;
}

int nearest_neighboor_grasp(Instance *inst) {
    DEBUG_COMMENT("greedy::nng", "start nearest neighboor");

    int rankings_index[inst->grasp_n_probabilities];
    double rankings_value[inst->grasp_n_probabilities];

    // Nearest Neighboor -------------------------------------------
    int current_node;
    for (int j = 1; j < inst->nnodes; j++) {
        // initialize rankings
        for (int i = 0; i < inst->grasp_n_probabilities; i++) {
            rankings_index[i] =
                -1;  // first component is the index of the element plus 1
            rankings_value[i] =
                INFINITY;  // second component is a random value between 0 and 1
        }
        // find the best #inst->grasp_n_probabilities neighboors
        current_node = inst->path[j - 1];
        double min_distance = INFINITY;
        double dist = -1;
        for (int k = j; k < inst->nnodes; k++) {
            dist = inst->distance_matrix[current_node * inst->nnodes + inst->path[k]];
            if (dist < rankings_value[inst->grasp_n_probabilities - 1])
                replace_if_better(rankings_index, rankings_value, inst->grasp_n_probabilities, k, dist);
        }

        // pick one of the bests
        int index = -1;
        double r = (double)rand() / (double)RAND_MAX;  // generate a random number between 1-100
        DEBUG_COMMENT("greedy::nng", "inst->grasp_probabilities %lf %lf %lf| r: %lf ",
                inst->grasp_probabilities[0], inst->grasp_probabilities[1], inst->grasp_probabilities[2], r);
        for (int i = 0; i < inst->grasp_n_probabilities; i++) {
            if (r <= inst->grasp_probabilities[i]) {
                index = i;
                break;
            }
        }
        DEBUG_COMMENT("greedy::nng", "%lf %lf %lf %lf -> %lf", r, rankings_value[0],
                rankings_value[1], rankings_value[2], rankings_value[index]);
        min_distance = rankings_value[index];
        swapPathPoints(inst, j, rankings_index[index]);
        addToTourLenght(inst,min_distance);

        DEBUG_COMMENT("greedy::nng",
                "tour length, starting from %d, with %d nodes = %lf", inst->path[0],
                j, inst->tour_length);
    }
    // complete the tour
    //
    //(*inst->tour_length) += inst->distance_matrix[inst->path[0] * inst->nnodes + inst->path[inst->nnodes - 1]];
    calculateTourLength(inst);
#ifndef PRODUCTION
    log_path(inst->path, inst->nnodes);
#endif
    return SUCCESS;
}

int extra_mileage(Instance *inst) {
    //--------------- FIND DIAMETER -------------------------------------------
    double max_distance = 0,tdist;
    int max_index = -1;

    for (int i = 1; i < inst->nnodes; i++) {
        tdist=getDistancePos(inst, 0, i);
        if (tdist > max_distance && tdist != INFINITY) {
            max_distance = tdist;
            max_index = i;
        }
    }
    DEBUG_COMMENT("greedy::Extra_mileage", "max distance = %lf, at index = %d",
            max_distance, max_index);
    swapPathPoints(inst, 1, max_index);

    //--------------- START SEARCH -------------------------------------------
    double tour_length = 2 * max_distance;
    DEBUG_COMMENT("greedy::Extra_mileage", "initial tour length = %lf",
            tour_length);
    int node_3[3] = {-1, -1, -1};
    double min = INFINITY;

    for (int i = 2; i < inst->nnodes; i++)  // starting with two inst->path "visited"
    {
        min = INFINITY;
        DEBUG_COMMENT("greedy::Extra_mileage", "start iteration %d", i);
        double new_triangular_sum;
        int is_close_edge = 0;

        // Check from all the inst->path to the following one
        for (int j = 0; j < i; j++) {
            for (int k = i; k < inst->nnodes; k++) {

                new_triangular_sum = getDistancePos(inst,j,k) + getDistancePos(inst, j + 1, k);
                if (new_triangular_sum < min) {
                    min = new_triangular_sum;
                    node_3[0] = j;
                    node_3[1] = j + 1;
                    node_3[2] = k;
                }
            }
        }

        for (int k = i; k < inst->nnodes; k++) {
            new_triangular_sum = getDistancePos(inst, 0, k) + getDistancePos(inst, i-1, k);

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
                inst->path[node_3[0]], inst->path[node_3[1]], inst->path[node_3[2]]);
        double new_cost = min - getDistancePos(inst, node_3[0], node_3[1]);
        tour_length += new_cost;
        DEBUG_COMMENT("greedy::Extra_mileage", "new cost-->%lf=%lf+%lf-%lf",
                new_cost,
                getDistancePos(inst, node_3[0], node_3[2]),
                getDistancePos(inst, node_3[1], node_3[2]),
                getDistancePos(inst, node_3[0], node_3[1])
        );
        DEBUG_COMMENT("greedy::Extra_mileage", "tour length-->%lf", inst->tour_length);

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
                inst->path[k] = inst->path[k - 1];  // Shift all elements to the right from j to i+1
            }
            inst->path[node_3[1]] = tmp;
        }
        DEBUG_COMMENT("greedy::Extra_mileage", "inst->path shifted");
    }

    // Put the saved value from position j into position i
    if (assertInst(inst))
        OUTPUT_COMMENT("greedy::extra_mileage", "found best tour lenght %lf",
                inst->tour_length);
    return SUCCESS;
}
