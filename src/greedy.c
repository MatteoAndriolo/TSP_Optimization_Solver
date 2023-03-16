#include <math.h>
#include "greedy.h"
#include "utils.h"

double nearest_neighboor(const double* distance_matrix, const int *path, int nnodes){
// nearest neighbor
        double tour_length = 0;
        int cur_node;
        int best_remaining;
        DEBUG_COMMENT("greedy::model_nearest_neighboor", "start nearest neighboor");
        for (int j = 1; j < nnodes - 1; j++)
        { 
            cur_node = path[j - 1];
            double min_distance = INFINITY;
            for (int k = j; k < nnodes; k++)
            {
                double dist = distance_matrix[cur_node * nnodes + path[k]];
                if (dist < min_distance) 
                {
                    min_distance = dist;
                    best_remaining = k;
                }
            }
            tour_length += min_distance;
            DEBUG_COMMENT("greedy::model_nearest_neighboor", "tour length, starting from %d, with %d nodes = %f", path[0], j, tour_length);
            swap(&path, j, best_remaining);
        }
        // complete the tour
        tour_length += distance_matrix[path[nnodes-1] * nnodes + path[0]];
        return tour_length;
}


void model_nearest_neighboor(Instance *inst)
{
    INFO_COMMENT("greedy::model_nearest_neighboor","Starts model nearest neighboor");
    double *distance_matrix = (double *)malloc(pow(inst->nnodes, 2) * sizeof(double));
    generate_distance_matrix(&distance_matrix, inst->nnodes, &inst->x, &inst->y, inst->integer_costs );
    DEBUG_COMMENT("greedy::model_nearest_neighboor", "distance matrix generated");

    // -----------  MAIN CYCLE ------------------------------------------------
    DEBUG_COMMENT("greedy::model_nearest_neighboor", "Starting main cycle");
    double tour_length;
    for (int i=0; i<inst->nnodes; i++) {
        // initialize array [0,1,2,3,4...,n] - stores final path
        int *nodes = malloc(inst->nnodes * sizeof(int));
        for (int j = 0; j < inst->nnodes; j++)
        {
            nodes[j] = j;
        }
        // set starting point at j (e.g. j=4 -> [4,1,2,3,0,5,...,n])
        swap(&nodes, 0, i);
        INFO_COMMENT("greedy::model_nearest_neighboor","starts from %d",i);
        tour_length=nearest_neighboor(&distance_matrix, &nodes, inst->nnodes);
        DEBUG_COMMENT("greedy::model_nearest_neighboor", "Best tour length starting from %d is %f", i, tour_length);

        //TODO grasp
        if (tour_length < inst->zbest) {
            inst->zbest = tour_length;
            inst->path_best = memcpy(nodes, inst->nnodes, sizeof(double));
        }
        
        OUTPUT_COMMENT("greedy::nearest_neighboor", "Optimal Tour lenght = %f", tour_length);
    }

    INFO_COMMENT("greedy::model_nearest_neighboor", "Optimal tour lenght found --> %f", tour_length);
    log_output(&inst);

    // Free memory
    free(distance_matrix);
}

void extra_mileage( Instance *inst)
{
    // Generate distance matrix --------
    DEBUG_COMMENT("greedy::Extra_mileage", "start extra mileage");
    double *distance_matrix = NULL;
    generate_distance_matrix(&distance_matrix, inst->nnodes, inst->x, inst->y, inst->integer_costs );
    DEBUG_COMMENT("greedy::Extra_mileage", "distance matrix generated");

    int nnodes=inst->nnodes; //TODO replace all inst-nodes with nnodes (should increase performance)
    int *nodes = malloc(nnodes * sizeof(int));
    int *remaining_nodes = malloc(nnodes * sizeof(int));
    
    for (int j = 0; j < nnodes; j++)
    {
        nodes[j] = j;
        remaining_nodes[j] = j;
    }

    /*----- SEARCH DIAMETER -----*/
    int row = 0;
    int col = 0;
    double max_distance = 0;
    int max_index = -1;
    for (int i=0; i< nnodes; i++){
        for (int j=i+1;nnodes; i++){
            if(distance_matrix[i*nnodes+j])
        }
    }
    for (int i = 1; i < inst->nnodes * inst->nnodes; i++)
    {
        if (distance_matrix[i] == INFINITY)
        {
            i = i * inst->nnodes + inst->nnodes; // 0 * 3 + 3 = 3 --> 3 * 1 + 3 = 6 --> 2 * 3 + 3 = 9 ecc...
        }

        if (max_distance < distance_matrix[i])
        {
            max_distance = distance_matrix[i];
            max_index = i;
        }

        row = (int)max_index / inst->nnodes;
        col = max_index % inst->nnodes;

        DEBUG_COMMENT("greedy::Extra_mileage", "max distance %f", max_distance);
        DEBUG_COMMENT("greedy::Extra_mileage", "max distance index %d", max_index);
        DEBUG_COMMENT("greedy::Extra_mileage", "correspond to node %d and %d", row < col ? row : col, row > col ? row : col);
    }

    int tour_length = 2 * max_distance;

    // swap the first 2 elements with the one with the max distance
    swap(&nodes, 0, row);
    swap(&nodes, 1, col);

    /*----- START SEARCH -----*/
    for (int i = 2; i < inst->nnodes; i++)
    {
        DEBUG_COMMENT("greedy::Extra_mileage", "start iteration %d", i);
        int index_j = -1;
        int node1, node2, node3;
        int min_index = -1;
        int best_nodes[3] = {-1, -1, -1};
        double best_value = 99999999999999999999.000;
        char nodes_str[1024] = "";
        for (int o = 0; o < inst->nnodes; o++)
        {
            char node_val[16] = "";
            sprintf(node_val, " |%d| ", nodes[o]);
            strcat(nodes_str, node_val);
        }
        DEBUG_COMMENT("greedy::Extra_mileage", "Nodes: %s", nodes_str);
        for (int j = i - 1; j - 1 >= 0; j--)
        { // node1 node2 adjacent, node3 completes the triangle
            node1 = nodes[j - 1];
            node2 = nodes[j];
            for (int k = i; k < inst->nnodes; k++)
            {
                node3 = nodes[k];
                if (distance_matrix[node1 * inst->nnodes + node3] + distance_matrix[node2 * inst->nnodes + node3] < best_value)
                {
                    best_value = distance_matrix[node1 * inst->nnodes + node3] + distance_matrix[node2 * inst->nnodes + node3];
                    memcpy(best_nodes, (int[]){node1, node2, node3}, sizeof(best_nodes));
                    min_index = k;
                    index_j = j;
                }
            }
        }
        DEBUG_COMMENT("greedy::Extra_mileage", "best value-->%f, best_triangele-->{%d,%d,%d}, min_index-->%d, index_j-->%d", best_value, node1, node2, node3, min_index, index_j);
        nodes[index_j] = best_nodes[2];
        for (int k = min_index; k > index_j; k--)
        {
            nodes[k] = nodes[k - 1];
        }
        nodes[index_j + 1] = best_nodes[1];

        tour_length += (best_value - distance_matrix[best_nodes[0] * inst->nnodes + best_nodes[1]]);
        DEBUG_COMMENT("greedy::Extra_mileage", "tour length-->%f", tour_length);
    }
    // find best between first and last
    /*node1 = nodes[0];
    node2 = nodes[i - 1];
    for (int k = i; k < inst->nnodes; k++)
    {
        node3 = nodes[k];
        if (distance_matrix[node1 * inst->nnodes + node3] + distance_matrix[node2 * inst->nnodes + node3] < best_value)
        {
            best_value = distance_matrix[node1 * inst->nnodes + node3] + distance_matrix[node2 * inst->nnodes + node3];
            memcpy(best_nodes, (int[]){node1, node2, node3}, sizeof(best_nodes));
            min_index = k;
        }
    }*/
    log_output(&inst);
    free(nodes);
    free(remaining_nodes);
}

void updated_extra_mileage(Instance *inst)
{
    DEBUG_COMMENT("greedy::Extra_mileage", "start extra mileage");
    double *distance_matrix = NULL;
    generate_distance_matrix(&distance_matrix, inst);
    DEBUG_COMMENT("greedy::Extra_mileage", "distance matrix generated");
    int *nodes = malloc(inst->nnodes * sizeof(int));
    int *remaining_nodes = malloc(inst->nnodes * sizeof(int));
    for (int j = 0; j < inst->nnodes; j++)
    {
        nodes[j] = j;
        remaining_nodes[j] = j;
    }

    DEBUG_COMMENT("greedy::Extra_mileage", "%lf", distance_matrix[1]);

    int row = 0;
    int col = 0;
    double max_distance = 0;
    int max_index = -1;
    for (int i = 1; i < inst->nnodes * inst->nnodes; i++)
    {
        if (distance_matrix[i] == INFINITY)
            i = i * inst->nnodes + inst->nnodes; //  0 * 3 + 3 = 3 --> 3 * 1 + 3 = 6 --> 2 * 3 + 3 = 9 ecc...

        if (max_distance < distance_matrix[i])
        {
            max_distance = distance_matrix[i];
            max_index = i;
        }

        row = (int)max_index / inst->nnodes;
        col = max_index % inst->nnodes;

        DEBUG_COMMENT("greedy::Extra_mileage", "max distance %f", max_distance);
        DEBUG_COMMENT("greedy::Extra_mileage", "max distance index %d", max_index);
        DEBUG_COMMENT("greedy::Extra_mileage", "row %d", row);
        DEBUG_COMMENT("greedy::Extra_mileage", "col %d", col);
    }

    double tour_length = 2 * max_distance;

    // swap the first 2 elements with the one with the max distance
    swap(&nodes, 0, row);
    swap(&nodes, 1, col);

    int count = 0;
    double min = 9999999999999999999.00000;
    int i = 2;
    int j = 2;
    int k = 0;
    int min_index = -1;
    // START SEARCH
    for (i = 2; i < inst->nnodes; i += count)
    {
        DEBUG_COMMENT("greedy::Extra_mileage", "i-->%d", i);
        count = 0;

        for (j = i - 1; j - 1 >= 0; j--)
        {
            int node1 = nodes[j];
            int node2 = nodes[j - 1];

            if (i + count < inst->nnodes)
            {
                min = distance_matrix[node1 * inst->nnodes + nodes[i + count]] + distance_matrix[node2 * inst->nnodes + nodes[i + count]];
                for (k = i + count; k < inst->nnodes; k++)
                {
                    int node3 = nodes[k];
                    if (distance_matrix[node1 * inst->nnodes + node3] + distance_matrix[node2 * inst->nnodes + node3] < min)
                    {
                        min = distance_matrix[node1 * inst->nnodes + node3] + distance_matrix[node2 * inst->nnodes + node3];
                        min_index = k;
                    }
                }

                DEBUG_COMMENT("greedy::Extra_mileage", "{j, j - 1} --> {%d,%d}, min index found and cost -->{%d,%f}, count-->%d, starting point k-->%d", j, j - 1, min_index, min, count + 1, count + i);

                int temp = nodes[j];
                nodes[j] = nodes[min_index];

                for (int f = min_index; f > j; f--)
                {
                    nodes[f] = nodes[f - 1];
                }
                nodes[j + 1] = temp;
                count++;
                tour_length = tour_length - distance_matrix[node1 * inst->nnodes + node2] + min;
                DEBUG_COMMENT("greedy::Extra_mileage", "tour length %f = %f - %f", tour_length, min, distance_matrix[node1 * inst->nnodes + node2]);

                char nodes_str[1024] = "";
                for (int o = 0; o < inst->nnodes; o++)
                {
                    char node_val[16] = "";
                    sprintf(node_val, " |%d| ", nodes[o]);
                    strcat(nodes_str, node_val);
                }
                DEBUG_COMMENT("greedy::Extra_mileage", "Nodes: %s", nodes_str);
            }
        }
    }
    free(nodes);
    free(remaining_nodes);
    tour_length = tour_length + max_distance; //+ max as well, it works
    OUTPUT_COMMENT("greedy::Extra_mileage", "tour length %f", tour_length);
}