#include "greedy.h"
#include "logger.h"
#include "utils.h"
#include "logger.h"
#include <stdlib.h>
#include <math.h>
#include "refinement.h"

void nearest_neighboor(const double *distance_matrix, int *path, int nnodes, double *tour_length)
{
    int current_node;
    int best_remaining = -1;
    DEBUG_COMMENT("greedy::nearest_neighboor", "start nearest neighboor");
    for (int j = 1; j < nnodes; j++)
    {
        current_node = path[j - 1];
        double min_distance = INFINITY;
        double dist = -1;
        for (int k = j; k < nnodes; k++)
        {
            dist = distance_matrix[current_node * nnodes + path[k]];
            if (dist < min_distance)
            {
                min_distance = dist;
                best_remaining = k;
            }
        }
        *tour_length += min_distance; // TODO implement grasp
        DEBUG_COMMENT("greedy::model_nearest_neighboor", "tour length, starting from %d, with %d nodes = %lf", path[0], j, *tour_length);
        swap(path, j, best_remaining);
    }
    // complete the tour
    (*tour_length) += distance_matrix[path[0] * nnodes + path[nnodes - 1]];

    int path_is_ok = assert_path(path, distance_matrix, nnodes, *tour_length);
    if (!path_is_ok)
    {
        ERROR_COMMENT("greedy::model_nearest_neighboor", "path is not ok");
        *tour_length = -1;
    }
}

void nearest_neighboor_grasp(const double *distance_matrix, int *path, const int nnodes, double *tour_length, const double *probabilities, const int n_prob)
{
    DEBUG_COMMENT("greedy::nng", "start nearest neighboor");

    int rankings_index[n_prob];
    double rankings_value[n_prob];

    // Nearest Neighboor -------------------------------------------
    int current_node;
    for (int j = 1; j < nnodes; j++)
    {
        // initialize rankings
        for (int i = 0; i < n_prob; i++)
        {
            rankings_index[i] = -1;       // first component is the index of the element plus 1
            rankings_value[i] = INFINITY; // second component is a random value between 0 and 1
        }
        // find the best #n_prob neighboors
        current_node = path[j - 1];
        double min_distance = INFINITY;
        double dist = -1;
        for (int k = j; k < nnodes; k++)
        {
            dist = distance_matrix[current_node * nnodes + path[k]];
            if (dist < rankings_value[n_prob - 1])
                replace_if_better(rankings_index, rankings_value, n_prob, k, dist);
        }

        // pick one of the bests
        int index = -1;
        double r = rand()/RAND_MAX; // generate a random number between 1-100
        DEBUG_COMMENT("greedy::nng", "probabilities %lf %lf %lf| r: %lf ", probabilities[0], probabilities[1], probabilities[2], r);
        for (int i = 0; i < n_prob; i++)
        {
            if (r <= probabilities[i])
            {
                index = i;
                break;
            }
        }
        DEBUG_COMMENT("greedy::nng", "%lf %lf %lf %lf -> %lf", r, rankings_value[0], rankings_value[1], rankings_value[2], rankings_value[index]);
        min_distance = rankings_value[index];
        swap(path, j, rankings_index[index]);

        (*tour_length) += min_distance;
        DEBUG_COMMENT("greedy::nng", "tour length, starting from %d, with %d nodes = %lf", path[0], j, *tour_length);
    }
    // complete the tour
    (*tour_length) += distance_matrix[path[0] * nnodes + path[nnodes - 1]];
    log_path(path, nnodes);
}

void extra_mileage(const double *distance_matrix, int *path, int nnodes, double *tour_length)
{
    //--------------- FIND DIAMETER -------------------------------------------
    double max_distance = 0;
    int max_index = -1;

    for (int i = 1; i < nnodes; i++)
    {
        if (distance_matrix[path[0] * nnodes + i] > max_distance &&
            distance_matrix[path[0] * nnodes + i] != INFINITY)
        {
            max_distance = distance_matrix[path[0] * nnodes + i];
            max_index = i;
        }
    }
    DEBUG_COMMENT("greedy::Extra_mileage", "max distance = %lf, at index = %d", max_distance, max_index);
    swap(path, 1, max_index);

    //--------------- START SEARCH -------------------------------------------
    *tour_length = 2 * max_distance;
    DEBUG_COMMENT("greedy::Extra_mileage", "initial tour length = %lf", tour_length);
    int node_3[3];
    double min = INFINITY;

    for (int i = 2; i < nnodes; i++) // starting with two path "visited"
    {
        min = INFINITY;
        DEBUG_COMMENT("greedy::Extra_mileage", "start iteration %d", i);
        double new_triangular_sum;
        int is_close_edge = 0;
        // Check from all the path to the following one

        for (int j = 0; j < i; j++)
        {
            for (int k = i; k < nnodes; k++)
            {
                new_triangular_sum = distance_matrix[path[j] * nnodes + path[k]] + distance_matrix[path[j + 1] * nnodes + path[k]];
                if (new_triangular_sum < min)
                {
                    min = new_triangular_sum;
                    node_3[0] = j;
                    node_3[1] = j + 1;
                    node_3[2] = k;
                }
            }
        }

        for (int k = i; k < nnodes; k++)
        {
            new_triangular_sum = distance_matrix[path[0] * nnodes + path[k]] + distance_matrix[path[i - 1] * nnodes + path[k]];
            if (new_triangular_sum < min)
            {
                min = new_triangular_sum;
                node_3[0] = 0;
                node_3[1] = i - 1;
                node_3[2] = k;
                is_close_edge = 1;
            }
        }

        // FOUND BEST NODE
        DEBUG_COMMENT("greedy::Extra_mileage", "best value-->%f, best_triangle indices -->{%d,%d,%d}", min, path[node_3[0]], path[node_3[1]], path[node_3[2]]);
        double new_cost = min - distance_matrix[path[node_3[0]] * nnodes + path[node_3[1]]];
        *tour_length += new_cost;
        DEBUG_COMMENT("greedy::Extra_mileage", "new cost-->%lf=%lf+%lf-%lf",
                      new_cost, distance_matrix[path[node_3[0]] * nnodes + path[node_3[2]]], distance_matrix[path[node_3[1]] * nnodes + path[node_3[2]]], distance_matrix[path[node_3[0]] * nnodes + path[node_3[1]]]);
        DEBUG_COMMENT("greedy::Extra_mileage", "tour length-->%lf", tour_length);
        // SWAP - ADJUST PATH ----------------------------------------------------
        // Save the value at position j in a temporary variable
        if (is_close_edge)
        {
            int tmp = path[node_3[2]]; // save node in index best
            for (int k = node_3[2]; k > node_3[1]; k--)
            {
                path[k] = path[k - 1];
            }
            path[node_3[1] + 1] = tmp;
        }
        else
        {
            int tmp = path[node_3[2]];
            for (int k = node_3[2]; k >= node_3[1]; k--)
            {
                path[k] = path[k - 1]; // Shift all elements to the right from j to i+1
            }
            path[node_3[1]] = tmp;
        }
        DEBUG_COMMENT("greedy::Extra_mileage", "path shifted");
    }

    // Put the saved value from position j into position i
    if (assert_path(path, distance_matrix, nnodes, *tour_length))
        OUTPUT_COMMENT("greedy::extra_mileage", "found best tour lenght %lf", tour_length);
}

// void updated_extra_mileage(Instance *inst)
// {
//     DEBUG_COMMENT("greedy::Extra_mileage", "start extra mileage");
//     double *distance_matrix = NULL;
//     generate_distance_matrix(&distance_matrix, inst);
//     DEBUG_COMMENT("greedy::Extra_mileage", "distance matrix generated");
//     int *nodes = malloc(inst->nnodes * sizeof(int));
//     int *remaining_nodes = malloc(inst->nnodes * sizeof(int));
//     for (int j = 0; j < inst->nnodes; j++)
//     {
//         nodes[j] = j;
//         remaining_nodes[j] = j;
//     }

//     DEBUG_COMMENT("greedy::Extra_mileage", "%lf", distance_matrix[1]);

//     int row = 0;
//     int col = 0;
//     double max_distance = 0;
//     int max_index = -1;
//     for (int i = 1; i < inst->nnodes * inst->nnodes; i++)
//     {
//         if (distance_matrix[i] == INFINITY)
//             i = i * inst->nnodes + inst->nnodes; //  0 * 3 + 3 = 3 --> 3 * 1 + 3 = 6 --> 2 * 3 + 3 = 9 ecc...

//         if (max_distance < distance_matrix[i])
//         {
//             max_distance = distance_matrix[i];
//             max_index = i;
//         }

//         row = (int)max_index / inst->nnodes;
//         col = max_index % inst->nnodes;

//         DEBUG_COMMENT("greedy::Extra_mileage", "max distance %f", max_distance);
//         DEBUG_COMMENT("greedy::Extra_mileage", "max distance index %d", max_index);
//         DEBUG_COMMENT("greedy::Extra_mileage", "row %d", row);
//         DEBUG_COMMENT("greedy::Extra_mileage", "col %d", col);
//     }

//     double tour_length = 2 * max_distance;

//     // swap the first 2 elements with the one with the max distance
//     swap(&nodes, 0, row);
//     swap(&nodes, 1, col);

//     int count = 0;
//     double min = INFINITY;
//     int i = 2;
//     int j = 2;
//     int k = 0;
//     int min_index = -1;
//     // START SEARCH
//     for (i = 2; i < inst->nnodes; i += count)
//     {
//         DEBUG_COMMENT("greedy::Extra_mileage", "i-->%d", i);
//         count = 0;

//         for (j = i - 1; j - 1 >= 0; j--)
//         {
//             int node1 = nodes[j];
//             int node2 = nodes[j - 1];

//             if (i + count < inst->nnodes)
//             {
//                 min = distance_matrix[node1 * inst->nnodes + nodes[i + count]] + distance_matrix[node2 * inst->nnodes + nodes[i + count]];
//                 for (k = i + count; k < inst->nnodes; k++)
//                 {
//                     int node3 = nodes[k];
//                     if (distance_matrix[node1 * inst->nnodes + node3] + distance_matrix[node2 * inst->nnodes + node3] < min)
//                     {
//                         min = distance_matrix[node1 * inst->nnodes + node3] + distance_matrix[node2 * inst->nnodes + node3];
//                         min_index = k;
//                     }
//                 }

//                 DEBUG_COMMENT("greedy::Extra_mileage", "{j, j - 1} --> {%d,%d}, min index found and cost -->{%d,%f}, count-->%d, starting point k-->%d", j, j - 1, min_index, min, count + 1, count + i);

//                 int temp = nodes[j];
//                 nodes[j] = nodes[min_index];

//                 for (int f = min_index; f > j; f--)
//                 {
//                     nodes[f] = nodes[f - 1];
//                 }
//                 nodes[j + 1] = temp;
//                 count++;
//                 tour_length = tour_length - distance_matrix[node1 * inst->nnodes + node2] + min;
//                 DEBUG_COMMENT("greedy::Extra_mileage", "tour length %f = %f - %f", tour_length, min, distance_matrix[node1 * inst->nnodes + node2]);

//                 char nodes_str[1024] = "";
//                 for (int o = 0; o < inst->nnodes; o++)
//                 {
//                     char node_val[16] = "";
//                     sprintf(node_val, " |%d| ", nodes[o]);
//                     strcat(nodes_str, node_val);
//                 }
//                 DEBUG_COMMENT("greedy::Extra_mileage", "Nodes: %s", nodes_str);
//             }
//         }
//     }
//     free(nodes);
//     free(remaining_nodes);
//     tour_length = tour_length + max_distance; //+ max as well, it works
//     OUTPUT_COMMENT("greedy::Extra_mileage", "tour length %f", tour_length);
// }