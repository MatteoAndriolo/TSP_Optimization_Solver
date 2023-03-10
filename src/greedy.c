#include <math.h>
#include "greedy.h"

inline void swap(int** arr, int i, int j) {
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
    // INITIALIZE nodes array and distance matrix
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

    // INITIALIZE diameter
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
        log_message(DEBUG , "greedy::Extra_mileage", "correspond to node %d and %d", row<col ? row:col, row>col ? row:col);
    }

    int tour_length=2*max_distance;

    //swap the first 2 elements with the one with the max distance
    swap(&nodes, 0, row);
    swap(&nodes, 1, col);

    /*----- START SEARCH -----*/
    for (int i = 2; i < inst->nnodes; i++){
        int node1,node2,node3;

        int min_index=-1;
        int best_nodes[3]={-1,-1,-1};
        double best_value=INFTY;
        for(int j=1; j<i;i++){ // node1 node2 adjacent, node3 completes the triangle
            node1=nodes[j-1]; 
            node2=nodes[j]; 
            for (int k = i; k < inst->nnodes; k++){ 
                node3=nodes[k];
                if (distance_matrix[node1 * inst->nnodes + node3] + distance_matrix[node2 * inst->nnodes + node3] < best_value){
                    best_value = distance_matrix[node1 * inst->nnodes + node3] + distance_matrix[node2 * inst->nnodes + node3];
                    memcpy(best_nodes, (int[]){node1,node2,node3}, sizeof(best_nodes));  
                    min_index = k;
                }
            }
        }
        //find best between first and last
        node1=nodes[0];
        node2=nodes[i-1];
        for (int k = i; k < inst->nnodes; k++){ 
            node3=nodes[k];
            if (distance_matrix[node1 * inst->nnodes + node3] + distance_matrix[node2 * inst->nnodes + node3] < best_value){
                best_value = distance_matrix[node1 * inst->nnodes + node3] + distance_matrix[node2 * inst->nnodes + node3];
                memcpy(best_nodes, (int[]){node1,node2,node3}, sizeof(best_nodes));  
                min_index = k;
            }
        }
        
        //insert node3 in position min_index in nodes
        for (int k = i; k > min_index; k--){         //check if position of replacement is correct, in particular >min_index
            nodes[k]=nodes[k-1];
        }
        nodes[min_index]=best_nodes[2];
        tour_length+=(best_value-distance_matrix[best_nodes[0] * inst->nnodes + best_nodes[1]]);

    }

}


void strange_extra_mileage(const Instance *inst) {
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
    int count = 0;
    double min = INFINITY;
    // START SEARCH
    for (int i = 2; i < inst->nnodes; i += count){
        
        for(int j = i; j - 2 >= 0 ; j =- 2){
            count = 0;
            min = INFINITY;
            int min_index=-1; 
            int node1=nodes[j - 1];
            int node2=nodes[j - 2];
            for (int k = i; k < inst->nnodes; k++){
                int node3 = nodes[k];
                if (distance_matrix[node1 * inst->nnodes + node3] + distance_matrix[node2 * inst->nnodes + node3] < min){
                    min = distance_matrix[node1 * inst->nnodes + node3] + distance_matrix[node2 * inst->nnodes + node3];
                    min_index = k;
                }
            }
            
            int temp = nodes[j-1];
            nodes[j-1] = nodes[min_index];
            for (int i = inst->nnodes - 1; i >= j; i--) {
                    nodes[i+1] = nodes[i];
            }
            nodes[i] = temp;
            count++;
        }

        tour_length += min;

    }

    tour_length += max_distance; //+ max as well, it works

}