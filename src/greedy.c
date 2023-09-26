#include "../include/greedy.h"

#include "../include/errors.h"
#include "../include/logger.h"
#include "../include/refinement.h"
#include "../include/utils.h"


int nearest_neighboor(Instance *inst) {
    INFO_COMMENT("nearest_neighboor", "start nearest neighboor");
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
    //TODO find diameter | farthest with lowest mean distance from other nodes
    // print_nodes(inst->x, inst->y, inst->nnodes);
    double max_distance = 0;
    double tdist;
    int max_index = -1;

    for (int i = 1; i < inst->nnodes; i++) {
        tdist=getDistancePos(inst, 0, i);
        if (tdist > max_distance && tdist != INFINITY) {
            max_distance = tdist;
            max_index = i;
        }
    }
    tdist=getDistancePos(inst, 0, inst->nnodes-1);
    if (tdist > max_distance && tdist != INFINITY) {
        max_distance = tdist;
        max_index = inst->nnodes-1;
    }

    DEBUG_COMMENT("greedy::Extra_mileage", "max_index = %d", max_index);
    DEBUG_COMMENT("greedy::Extra_mileage", "node_1=0 -> node %d --> (x,y)=%lf, %lf",inst->path[0], inst->x[0], inst->y[0]);
    DEBUG_COMMENT("greedy::Extra_mileage", "node_2=%d --> node %d --> (x,y)=%lf, %lf",max_index, inst->path[max_index], inst->x[inst->path[1]], inst->y[inst->path[1]]);
    DEBUG_COMMENT("greedy::Extra_mileage", "diameter = %lf", max_distance);
    swapPathPoints(inst, 1, max_index);
    setTourLenght(inst, 2 * max_distance);
    DEBUG_COMMENT("greedy::Extra_mileage", "tour length = %lf", inst->tour_length);
    //write nodes

    //--------------- START SEARCH -------------------------------------------
    DEBUG_COMMENT("greedy::Extra_mileage", "initial tour length = %lf",
            inst->tour_length);
    int node_3[3] = {-1, -1, -1};
    double min_nts = INFINITY;

    // add all the remaining nodes -> starts from current number of nodes -> 2(diameter)
    // check for all remaining node all the best possible positions in between the current nodes
    for (int i = 2; i < inst->nnodes; i++)
    {
        min_nts = INFINITY;
        DEBUG_COMMENT("greedy::Extra_mileage", "start iteration %d", i);
        double new_triangular_sum;
        bool is_close_edge = false;

        // Check from all the inst->path to the following one
        for (int j = 0; j < i-1; j++) {
            for (int k = i; k < inst->nnodes; k++) {
                new_triangular_sum = getDistancePos(inst,j,k) + getDistancePos(inst, j + 1, k);
                if (new_triangular_sum < min_nts) {
                    min_nts = new_triangular_sum;
                    node_3[0] = j;
                    node_3[1] = j + 1;
                    node_3[2] = k;
                }
            }
        }

        for (int k = i; k < inst->nnodes; k++) {
            new_triangular_sum = getDistancePos(inst, 0, k) + getDistancePos(inst, i-1, k);
            if (new_triangular_sum < min_nts) {
                min_nts = new_triangular_sum;
                node_3[0] = 0;
                node_3[1] = i - 1;
                node_3[2] = k;
                is_close_edge = true;
            }
        }
        double new_cost = min_nts - getDistancePos(inst, node_3[0], node_3[1]);

        addToTourLenght(inst,new_cost);

        // FOUND BEST NODE
        //print triangular sum calculation
        DEBUG_COMMENT("greedy::Extra_mileage","--------------------------");
        DEBUG_COMMENT("greedy::Extra_mileage", "nts = %lf + %lf", getDistancePos(inst, node_3[0], node_3[2]), getDistancePos(inst, node_3[1], node_3[2]));
        DEBUG_COMMENT("greedy::Extra_mileage", "new_cost = nts - old | %lf = %lf - %lf",
                new_cost, min_nts, getDistancePos(inst, node_3[0], node_3[1]));
        DEBUG_COMMENT("greedy::Extra_mileage",
                "best value-->%lf, best_triangle indices -->{%d,%d,%d}", min_nts,
                node_3[0], node_3[1], node_3[2]);
        DEBUG_COMMENT("greedy::Extra_mileage",
                "best value-->%lf, best_triangle nodes -->{%d,%d,%d}", min_nts,
                inst->path[node_3[0]], inst->path[node_3[1]], inst->path[node_3[2]]);
        DEBUG_COMMENT("greedy::Extra_mileage","best triangle nodes -->{%d,%d,%d}", node_3[0], node_3[1], node_3[2]);
        DEBUG_COMMENT("greedy::Extra_mileage", "coordinates of nodes: ");
        DEBUG_COMMENT("greedy::Extra_mileage", "node_3[0]=%d -> node %d --> (x,y)=%lf, %lf",node_3[0], inst->path[node_3[0]], inst->x[inst->path[node_3[0]]], inst->y[inst->path[node_3[0]]]);
        DEBUG_COMMENT("greedy::Extra_mileage", "node_3[1]=%d -> node %d --> (x,y)=%lf, %lf",node_3[1], inst->path[node_3[1]], inst->x[inst->path[node_3[1]]], inst->y[inst->path[node_3[1]]]);
        DEBUG_COMMENT("greedy::Extra_mileage", "node_3[2]=%d -> node %d --> (x,y)=%lf, %lf",node_3[2], inst->path[node_3[2]], inst->x[inst->path[node_3[2]]], inst->y[inst->path[node_3[2]]]);

        if (new_cost < 0) {
            ERROR_COMMENT("greedy::Extra_mileage", "new cost is negative");
            return FAILURE;
        }else if (new_cost == 0) {
            DEBUG_COMMENT("greedy::Extra_mileage", "new cost is zero");
        }

        DEBUG_COMMENT("greedy::Extra_mileage", "new cost-->%lf=%lf+%lf-%lf",
                new_cost,
                getDistancePos(inst, node_3[0], node_3[2]),
                getDistancePos(inst, node_3[1], node_3[2]),
                getDistancePos(inst, node_3[0], node_3[1])
        );
        //print coordinates of nodes

        DEBUG_COMMENT("greedy::Extra_mileage", "last tour length-->%lf", inst->tour_length);

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
