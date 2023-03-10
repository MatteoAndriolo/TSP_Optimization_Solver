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

void generate_distance_matrix(double ** matrix, Instance* inst){
    log_message(DEBUG , "greedy::generate_distance_matrix", "Generating distance matrix");
    ffflush();
    
    double *mat = (double*) malloc(pow(inst->nnodes,2) * sizeof(double));
    for (int i = 0; i <= inst->nnodes; i++){
        for (int j = 0; j <= inst->nnodes; j++){ 
            log_message(DEBUG , "greedy::generate_distance_matrix", "position %d", i*inst->nnodes + j );
            ffflush();
            (mat)[i * inst->nnodes + j] = distance(inst->x[i], inst->y[i], inst->x[j], inst->y[j]);
        }
    }
    log_message(DEBUG , "greedy::generate_distance_matrix", "finised generating distance matrix");
    ffflush();
    for (int i = 0; i < inst->nnodes ; i++){
        (mat)[i * inst->nnodes + i] = INFINITY;
    }
    log_message(DEBUG , "greedy::generate_distance_matrix", "added infinity");
    *matrix = mat;
    log_message(DEBUG , "greedy::generate_distance_matrix", "end");
}

void model_nearest_neighboor(Instance *inst) {
    double * distance_matrix=NULL;
    generate_distance_matrix(&distance_matrix, inst);
    
    /* TODO: remove comment
    for (int i=0; i<inst->nnodes; i++) {
        // initialize array [0,1,2,3,4...,n]
    */
        int * nodes = malloc(inst->nnodes * sizeof(int));
        for (int j=0; j<inst->nnodes; j++) {
            nodes[j] = j;
        }
    /*  TODO: remove comments
        // set starting point 4 -> [4,1,2,3,0,5,...,n]
        swap(&nodes, 0, i);    
    */
        // nearest neighbor
        double tour_length = 0;
        int cur_node;
        int best_remaining;
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

        // free memory
        free(nodes);
    /* TODO remove comments
    }
    */
           
}

void extra_mileage(Instance *inst) {
   
    log_message(DEBUG , "greedy::Extra_mileage", "start extra mileage");
    ffflush();
    double * distance_matrix=NULL;
    generate_distance_matrix(&distance_matrix, inst);
    int tour_length = 0;
    log_message(DEBUG , "greedy::Extra_mileage", "distance matrix generated");
    int * nodes = malloc(inst->nnodes * sizeof(int));
    for (int j=0; j < inst->nnodes; j++) {
            nodes[j] = j;
    }
    log_message(DEBUG , "greedy::Extra_mileage", "nodes array generated");
    
    int row = 0;
    int col = 0;
    for (int i = 1; i < inst->nnodes * inst->nnodes; i++ ){
        double max = distance_matrix[i];
        int max_index = i;
        if (distance_matrix[i] == INFINITY){
            i = i * inst->nnodes + inst->nnodes;//example with 3 nodes
                                                // 0 * 3 + 3 = 3 --> 3 * 1 + 3 = 6 --> 2 * 3 + 3 = 9 ecc...
        }

        if (max < distance_matrix[i]){
            max = distance_matrix[i];
            max_index = i;
        }
       
        row = (int)max_index / inst->nnodes;
        col = max_index % inst->nnodes;

        log_message(DEBUG , "greedy::Extra_mileage", "max distance %f", max);
        log_message(DEBUG , "greedy::Extra_mileage", "max distance index %d", max_index);
        log_message(DEBUG , "greedy::Extra_mileage", "row %d", row);
        log_message(DEBUG , "greedy::Extra_mileage", "col %d", col);
        ffflush();

    }

    //swap the first 2 elements with the one with the max distance
    swap(&nodes, 0, row);
    swap(&nodes, 1, col);

    for (int i = 2; i < inst->nnodes; i++){
        double min = INFINITY;
        int min_index = 0;
        for (int j = i; j < inst->nnodes; j++){
            if (min > distance_matrix[nodes[i-1] * inst->nnodes + nodes[j]] + distance_matrix[nodes[i-2] * inst->nnodes + nodes[j]]){
                min = distance_matrix[nodes[i-1] * inst->nnodes + nodes[j]] + distance_matrix[nodes[i-2] * inst->nnodes + nodes[j]];
                min_index = j;
            }
        }

        swap(&nodes, i, min_index);
        tour_length += min;

    }

    tour_length += distance_matrix[nodes[inst->nnodes-1] * inst->nnodes + nodes[0]]; //+ max as well, it works

    free(nodes);

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