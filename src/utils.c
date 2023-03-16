#include "utils.h"
#include <math.h>


inline void swap(int *arr, int i, int j)
{
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}


inline double distance_euclidean(const double* x1, const double* y1, const double* x2, const double* y2)
{
    double dx = x1 - x2;
    double dy = y1 - y2;
    return sqrt(dx * dx + dy * dy);
}
inline double distance_euclidean_square(const double* x1, const double* y1, const double* x2, const double* y2)
{
    double dx = x1 - x2;
    double dy = y1 - y2;
    return dx * dx + dy * dy;
}


void generate_distance_matrix(double *matrix, const int nnodes, const double *x, const double *y, int round ){
    //TODO when genereting round cost should we use integer instead of double?
    DEBUG_COMMENT("greedy::generate_distance_matrix", "Generating distance matrix");
    for (int i = 0; i <= nnodes; i++)
    {
        for (int j = 0; j <= nnodes; j++)
        {
            matrix[i * nnodes + j] = distance_euclidean(x[i], y[i], x[j], y[j]);
        }
    }
    for (int i = 0; i < nnodes; i++)
    {
        matrix[i * nnodes + i] = INFINITY;
    }
    DEBUG_COMMENT("greedy::generate_distance_*matrix", "Finised generating distance *matrixrix");
}