#include "greedy.h"
#include "logger.h"
#include "utils.h"
#include "logger.h"
#include <stdlib.h>
#include <math.h>
#include "refinement.h"

void nearest_neighboor(double *distance_matrix, int *path, int nnodes, int starting_node, double *tour_length)
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
}

void model_nearest_neighboor(Instance *inst, int instances)
{
    //----------------- DISTANCE MATRIX ----------------------------------------------------
    INFO_COMMENT("greedy::model_nearest_neighboor", "Starts model nearest neighboor");
    double *distance_matrix = (double *)malloc(pow(inst->nnodes, 2) * sizeof(double));
    generate_distance_matrix(&distance_matrix, inst->nnodes, inst->x, inst->y, inst->integer_costs);
    log_distancematrix(distance_matrix, inst->nnodes);
    //-------------- STARTING NODES -------------------------------------------------------
    DEBUG_COMMENT("greedy::model_nearest_neighboor", "distance matrix generated");
    int *starting_nodes = (int *)malloc(sizeof(int) * instances);
    generate_random_starting_nodes(starting_nodes, instances, inst->randomseed);
    DEBUG_COMMENT("greedy::model_nearest_neighboor", "starting nodes generated");
    //-------------- INIZIALIZE THE ARRAY ---------------------------------------------
    int *nodes = malloc(inst->nnodes * sizeof(int));
    for (int j = 0; j < inst->nnodes; j++)
    {
        nodes[j] = j;
    }
    // -----------  CALLING INSTANCES ------------------------------------------------
    for (int y = 0; y < instances; y++)
    {
        set_starting_node(nodes, starting_nodes[y], inst->nnodes);
        // -----------  MAIN CYCLE ------------------------------------------------
        DEBUG_COMMENT("greedy::model_nearest_neighboor", "Starting main cycle");
        double tour_length;

      
            // initialize array [0,1,2,3,4...,n] - stores final path

            // set starting point at j (e.g. j=4 -> [4,1,2,3,0,5,...,n])
            //swap(nodes, 0, i);
            INFO_COMMENT("greedy::model_nearest_neighboor", "Starting from %d", i);
            tour_length = 0;
            nearest_neighboor(distance_matrix, nodes, inst->nnodes, i, &tour_length);

            DEBUG_COMMENT("greedy::model_nearest_neighboor", "Best tour length starting from %d is %f", i, tour_length);
            int path_is_ok = assert_path(nodes, distance_matrix, inst->nnodes, tour_length);
            if (path_is_ok && tour_length < inst->zbest)
            {
                inst->zbest = tour_length;
                // memcpy(inst->path_best, nodes, sizeof(double) * inst->nnodes); //FIXME trapass border
            }
            OUTPUT_COMMENT("greedy::nearest_neighboor", "Optimal Tour lenght = %f", tour_length);
            log_output(inst->model_type, i, inst->zbest, inst->timelimit, inst->randomseed, inst->nnodes, inst->input_file);
            log_path(nodes, inst->nnodes);

        INFO_COMMENT("greedy::model_nearest_neighboor", "Optimal tour lenght found --> %f", tour_length);

        // TODO Free memory
        //  free(distance_matrix);
        //  free(nodes);
    }
}

void extra_mileage(Instance *inst, int instances)
{
    // -----------------------GENERATE DISTANCE MATRIX -------------------------------
    DEBUG_COMMENT("greedy::Extra_mileage", "start extra mileage");
    double *distance_matrix = (double *)malloc(pow(inst->nnodes, 2) * sizeof(double));
    generate_distance_matrix(&distance_matrix, inst->nnodes, inst->x, inst->y, inst->integer_costs);
    DEBUG_COMMENT("greedy::Extra_mileage", "distance matrix generated");
    //-------------- STARTING NODES -------------------------------------------------------
    DEBUG_COMMENT("greedy::model_nearest_neighboor", "distance matrix generated");
    int *starting_nodes = (int *)malloc(sizeof(int) * instances);
    generate_random_starting_nodes(starting_nodes, instances, inst->randomseed);
    DEBUG_COMMENT("greedy::model_nearest_neighboor", "starting nodes generated");
    //-------------- INIZIALIZE THE ARRAY ---------------------------------------------
    int *nodes = malloc(inst->nnodes * sizeof(int));
    for (int j = 0; j < inst->nnodes; j++)
    {
        nodes[j] = j;
    }
    // -----------  CALLING INSTANCES ------------------------------------------------
    for (int y = 0; y < instances; y++)
    {
        check_path(nodes, starting_nodes[y], inst->nnodes);
        // -----------  MAIN CYCLE ------------------------------------------------
        //--------------- FIND DIAMETER -------------------------------------------
        double max_distance = 0;
        int max_index = -1;
        for (int i = 0; i < pow(inst->nnodes, 2); i++)
        {
                if(distance_matrix[nodes[0] * inst->nnodes + i] > max_distance && 
                    distance_matrix[nodes[0] * inst->nnodes + i] != INFINITY)
                {
                    max_distance = distance_matrix[nodes[0] * inst->nnodes + i];
                    max_index = i;
                }
        }

        swap(nodes, 1, max_index);

        /*----- START SEARCH -----*/
        int tour_length = 2 * max_distance;
        int node_3[3];
        node_3[0] = -1;
        node_3[1] = -1;
        node_3[2] = -1;
        double best_triangular_sum = INFINITY;
        for (int i = 2; i < inst->nnodes; i++) // starting with two nodes "visited"
        {
            DEBUG_COMMENT("greedy::Extra_mileage", "start iteration %d", i);
            node_3[0] = nodes[i - 2];
            node_3[1] = nodes[i - 1];

            best_triangular_sum = distance_matrix[node_3[0] * inst->nnodes + node_3[1]];
            double new_triangular_sum;
            // Check from all the nodes to the following one
            for (int j = 0; j < i - 1; j++)
            {
                for (int k = i; k < inst->nnodes; k++)
                {
                    new_triangular_sum = distance_matrix[node_3[0] * inst->nnodes + j] + distance_matrix[node_3[1] * inst->nnodes + j];
                    if (new_triangular_sum < best_triangular_sum)
                    {
                        best_triangular_sum = new_triangular_sum;
                        node_3[0] = j;
                        node_3[1] = j + 1;
                        node_3[2] = k;
                    }
                }
            }
            // Check also substitution for the edge that closes the cycle
            for (int k = i; k < inst->nnodes; k++)
            {
                new_triangular_sum = distance_matrix[nodes[0] * inst->nnodes + k] + distance_matrix[nodes[i - 1] * inst->nnodes + k];
                if (new_triangular_sum < best_triangular_sum)
                {
                    best_triangular_sum = new_triangular_sum;
                    node_3[0] = 0;
                    node_3[1] = i - 1;
                    node_3[2] = k;
                }
            }

            // FOUND BEST NODE
            DEBUG_COMMENT("greedy::Extra_mileage", "best value-->%f, best_triangle indices -->{%d,%d,%d}", best_triangular_sum, node_3[0], node_3[1], node_3[2]);
            tour_length += new_triangular_sum - distance_matrix[nodes[node_3[0]] * inst->nnodes + nodes[node_3[1]]]; // FIXME tourlenght
            // UPDATE THE PATH
            swap_and_shift(nodes, nodes[node_3[2]], nodes[node_3[1]], inst->nnodes);
            DEBUG_COMMENT("greedy::Extra_mileage", "tour length-->%lf", tour_length);
        }
        if (assert_path(nodes, distance_matrix, inst->nnodes, tour_length))
            OUTPUT_COMMENT("greedy::extra_mileage", "found best tour lenght %lf", tour_length);
        else
        {
        }
        log_output(inst->model_type, nodes[0], tour_length, inst->timelimit, inst->randomseed, inst->nnodes, inst->input_file);
        free(nodes);
    }
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