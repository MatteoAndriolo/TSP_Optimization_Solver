#include "utils.h"
#include <math.h>

//TODO: inline removed
void swap(int *arr, int i, int j)
{
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}

//TODO: inline removed
double distance_euclidean(const double* x1, const double* y1, const double* x2, const double* y2)
{
    double dx = x1 - x2;
    double dy = y1 - y2;
    return sqrt(dx * dx + dy * dy);
}
//TODO: inline removed
double distance_euclidean_square(const double* x1, const double* y1, const double* x2, const double* y2)
{
    double dx = x1 - x2;
    double dy = y1 - y2;
    return dx * dx + dy * dy;
}


double *generate_distance_matrix(double *matrix, const int nnodes, const double *x, const double *y, int round ){
    //TODO when genereting round cost should we use integer instead of double?
    DEBUG_COMMENT("greedy::generate_distance_matrix", "Generating distance matrix");
    for (int i = 0; i <= nnodes; i++)
    {
        for (int j = 0; j <= nnodes; j++)
        {
            matrix[i * nnodes + j] = distance_euclidean(&x[i], &y[i], &x[j], &y[j]);
        }
    }
    for (int i = 0; i < nnodes; i++)
    {
        matrix[i * nnodes + i] = INFINITY;
    }
    DEBUG_COMMENT("greedy::generate_distance_*matrix", "Finised generating distance *matrixrix");
    return &matrix;
}

void generate_path(int * path, int starting_node, int num_nodes){
    for (int i = 0; i < num_nodes - starting_node; i++)
    {
        path[i] = i + starting_node;
    }
    for (int i = num_nodes - starting_node; i < num_nodes; i++)
    {
        path[i] = i - (num_nodes - starting_node);
    }
}

int assert_path(const int* path,const double* distance_matrix, int nnodes, double tour_length){
    // all nodes used
    int check_nnodes=((nnodes-1)*(nnodes-2))/2;
    for(int i=0; i<nnodes; check_nnodes-=path[i++]);

    // tour length correct
    double check_tour_length=distance_matrix[path[0]*nnodes+path[nnodes-1]];
    for(int i=0; i<nnodes-1; check_tour_length+=distance_matrix[path[i]*nnodes+path[++i]]);

    
    if (check_nnodes!=0 || check_tour_length!=tour_length){
        ERROR_COMMENT("assert_path", "assert_path failed: check_nnodes=%d, check_tour_length=%lf, nnodes=%d, tour_length=%lf\n", check_nnodes, check_tour_length, tour_length);
        return 0;
    }
    return 1;
}