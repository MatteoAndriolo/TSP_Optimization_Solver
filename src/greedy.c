#include <math.h>
#include "greedy.h"

void swap(int** arr, int i, int j) {
    int temp = (*arr)[i];
    (*arr)[i] = (*arr)[j];
    (*arr)[j] = temp;
}

/* Calculates the Euclidean distance between two points */
inline double distance(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    return sqrt(dx*dx + dy*dy);
}

void generate_distance_matrix(double **matrix, const Instance* inst){
    log_message(DEBUG , "greedy::generate_distance_matrix", "Generating distance matrix");
    
    *matrix= (double*) malloc(pow(inst->nnodes,2) * sizeof(double));
    log_message(DEBUG, "greedy::nvwfibodb", "pow: %lf", pow(inst->nnodes, 2));
    ffflush();
    for (int i = 0; i <= inst->nnodes; i++){
        for (int j = 0; j <= inst->nnodes; j++){ 
            //log_message(DEBUG , "greedy::generate_distance_*matrixrix", "position %d", i*inst->nnodes + j );
            ffflush();
            (*matrix)[i * inst->nnodes + j] = distance(inst->x[i], inst->y[i], inst->x[j], inst->y[j]);
        }
    }
    log_message(DEBUG , "greedy::generate_distance_*matrixrix", "finised generating distance *matrixrix");
    ffflush();
    
    for (int i = 0; i < inst->nnodes ; i++){
        (*matrix)[i * inst->nnodes + i] = INFINITY;
    }
    log_message(DEBUG , "greedy::generate_distance_*matrixrix", "added infinity");
    ffflush();
}

void model_nearest_neighboor(const Instance *inst) {
    double * distance_matrix=NULL;
    generate_distance_matrix(&distance_matrix, inst);
    log_message(DEBUG,"greedy::model_nearest_neighboor", "distance matrix generated");  
    /* TODO: remove comment
    for (int i=0; i<inst->nnodes; i++) {
        // initialize array [0,1,2,3,4...,n]
    */
    log_message(DEBUG , "greedy::model_nearest_neighboor", "%lf", distance_matrix[1]);
        int * nodes = malloc(inst->nnodes * sizeof(int));
        for (int j=0; j<inst->nnodes; j++) {
            nodes[j] = j;
        }
    /*  TODO: remove comments
        // set starting point 4 -> [4,1,2,3,0,5,...,n]
        swap(&nodes, 0, i);    
    */
        // nearest neighbor
        log_message(DEBUG,"greedy::model_nearest_neighboor", "start nearest neighboor");
        double tour_length = 0;
        int cur_node;
        int best_remaining;
        log_message(DEBUG , "greedy::model_nearest_neighboor", "start nearest neighboor");
        for (int j = 1; j < inst->nnodes-1; j++) {  // -1 because we don't need to check the last node
            cur_node=nodes[j-1];
            double min_distance = INFINITY;
            for (int k = j; k < inst->nnodes; k++) {
                double dist = distance_matrix[cur_node * inst->nnodes + nodes[k]]; 
                if (dist < min_distance) {
                    min_distance = dist;
                    best_remaining = k;
                }
            }
            tour_length += min_distance;
            swap(&nodes, j, best_remaining);
        }
        // complete the tour
        tour_length += distance_matrix[nodes[inst->nnodes-1] * inst->nnodes + nodes[0]];
        log_message(DEBUG , "greedy::model_nearest_neighboor", "tour length %d", tour_length);
        // update best tour if necessary
        //update_best_solution();
        /*
        if (tour_length < inst->best_tour_length) {
            inst->best_tour_length = tour_length;
            for (int j = 0; j < inst->nnodes; j++) {
                inst->best_tour[j] = nodes[j];
            }
        }
        */

    /* TODO remove comments
    }
    */
    log_message(DEBUG , "greedy::model_nearest_neighboor", "free nodes");
}

void extra_mileage(const Instance *inst) {
    log_message(DEBUG , "greedy::Extra_mileage", "start extra mileage");
    double * distance_matrix=NULL;
    generate_distance_matrix(&distance_matrix, inst);
    log_message(DEBUG , "greedy::Extra_mileage", "distance matrix generated");
    int * nodes = malloc(inst->nnodes * sizeof(int));
    int * remaining_nodes = malloc(inst->nnodes * sizeof(int));
    for (int j=0; j < inst->nnodes; j++) {
            nodes[j] = j;
            remaining_nodes[j] = j;
    }
    log_message(DEBUG , "greedy::Extra_mileage", "nodes array generated");
    log_message(DEBUG , "greedy::Extra_mileage", "%lf",distance_matrix[1]);

    int row = 0;
    int col = 0;
    double max_distance=0;
    int max_index=-1;
    for (int i = 1; i < inst->nnodes * inst->nnodes; i++ ){
        if (distance_matrix[i] == INFINITY){
            i = i * inst->nnodes + inst->nnodes;//example with 3 nodes
                                                // 0 * 3 + 3 = 3 --> 3 * 1 + 3 = 6 --> 2 * 3 + 3 = 9 ecc...
        }

        if (max_distance < distance_matrix[i]){
            max_distance = distance_matrix[i];
            max_index = i;
        }
       
        row = (int)max_index / inst->nnodes;
        col = max_index % inst->nnodes;

        log_message(DEBUG , "greedy::Extra_mileage", "max distance %f", max_distance);
        log_message(DEBUG , "greedy::Extra_mileage", "max distance index %d", max_index);
        log_message(DEBUG , "greedy::Extra_mileage", "row %d", row);
        log_message(DEBUG , "greedy::Extra_mileage", "col %d", col);
        ffflush();

    }

    int tour_length=2*max_distance;

    //swap the first 2 elements with the one with the max distance
    swap(&nodes, 0, row);
    swap(&nodes, 1, col);

    // START SEARCH
    for (int i = 2; i < inst->nnodes; i++){
        double min = INFINITY;
        int min_index=-1;

        for(int j=1; j<i;i++){ // node1 node2 adjacent, node3 completes the triangle
            int node1=nodes[j-1];
            int node2=nodes[j];
            for (int k = i; k < inst->nnodes; k++){
                int node3=nodes[k];
                if (distance_matrix[node1 * inst->nnodes + node3] + distance_matrix[node2 * inst->nnodes + node3] < min){
                    min = distance_matrix[node1 * inst->nnodes + node3] + distance_matrix[node2 * inst->nnodes + nodes3];
                    min_index = k;
                }
            }
            // find distance with nodes not visited
                // altro array con nodi rimanenti
        }
        //find best between first and last



        swap(&nodes, i, min_index);
        tour_length += min;

    }

    tour_length += distance_matrix[nodes[inst->nnodes-1] * inst->nnodes + nodes[0]]; //+ max as well, it works

}




/*
nearest neighbot:
	generate distance matrix
		eucllidean distance
	initialize array with all nodes
	swap starting point with the first one
	iterate
		put in i+1 closest node to node i
			get minimum distance in (i+1,n) nodes
	end when i=n-1; 
	add distance between starting node and the last one
*/