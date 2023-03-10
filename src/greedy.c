#include "greedy.h"
#include <math.h>

void swap(int** arr, int i, int j) {
    int temp = (*arr)[i];
    (*arr)[i] = (*arr)[j];
    (*arr)[j] = temp;
}

/* Calculates the Euclidean distance between two points */
inline double distance(double x1, double y1, double x2, double y2) {
    log_message(DEBUG , "greedy::distance", "in");
    ffflush();
    double dx = x1 - x2;
    double dy = y1 - y2;
    log_message(DEBUG , "greedy::distance", "end");
    ffflush();
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
            (mat)[i * inst->nnodes +j] = distance(inst->x[i], inst->y[i], inst->x[j], inst->y[j]);
        }
    }
    for (int i = 0; i < inst->nnodes ; i++){
        (mat)[i * inst->nnodes + i] = INFINITY;
    }
    log_message(DEBUG , "greedy::generate_distance_matrix", "finised generating distance matrix");
}

void model_nearest_neighboor(Instance *inst) {
    double * distance_matrix=NULL;
    generate_distance_matrix(&distance_matrix, inst);
    
    /* TODO remove comment
    for (int i=0; i<inst->nnodes; i++) {
        // initialize array [0,1,2,3,4...,n]
    */
        int * nodes = malloc(inst->nnodes * sizeof(int));
        for (int j=0; j<inst->nnodes; j++) {
            nodes[j] = j;
        }
    /*  TODO remove comments
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