#include "utils.h"

void swap(int *arr, int i, int j)
{
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}

void swap_and_shift(int *arr, int i, int j, int n)
{
    int temp = arr[j]; // Save the value at position j in a temporary variable
    for (int k = j; k > i; k--)
    {
        arr[k] = arr[k - 1]; // Shift all elements to the right from j to i+1
    }
    arr[i] = temp; // Put the saved value from position j into position i
}

inline double distance_euclidean(double x1, double y1, double x2, double y2)
{
    double dx = x1 - x2;
    double dy = y1 - y2;
    return sqrt(dx * dx + dy * dy);
}
inline double distance_euclidean_square(double x1, double y1, double x2, double y2)
{
    double dx = x1 - x2;
    double dy = y1 - y2;
    return dx * dx + dy * dy;
}

void generate_distance_matrix(double **matrix, const int nnodes, const double *x, const double *y, int r)
{
    DEBUG_COMMENT("greedy::generate_distance_matrix", "Generating distance matrix");
    for (int i = 0; i < nnodes; i++)
    {
        for (int j = 0; j < nnodes; j++)
        {
            (*matrix)[i * nnodes + j] = round(distance_euclidean(x[i], y[i], x[j], y[j]));
        }
    }
    for (int i = 0; i < nnodes; i++)
    {
        (*matrix)[i * nnodes + i] = INFINITY;
    }
    DEBUG_COMMENT("greedy::generate_distance_*matrix", "Finised generating distance *matrixrix");
}

void set_starting_node(int *path, int starting_node, int num_nodes)
{
    for (int i = 0; i < num_nodes; i++)
    {
        if (path[i] == starting_node)
        {
            swap(path, i, 0);
            return;
        }
    }
    ERROR_COMMENT("set_starting_node", "Starting node not found");
}

int assert_path(const int *path, const double *distance_matrix, const int nnodes,const double tour_length)
{
    // all nodes used
    int check_nnodes = (nnodes * (nnodes - 1)) / 2;
    for (int i = 0; i < nnodes; check_nnodes -= path[i++]);

    // tour length correct
    double check_tour_length = 0; // distance_matrix[path[0] * nnodes + path[nnodes - 1]];
    for (int i = 0; i < nnodes-1 ; i++)
    {
        check_tour_length += distance_matrix[path[i] * nnodes + path[i + 1]];
    }
    check_tour_length += distance_matrix[path[0] * nnodes + path[nnodes - 1]];

    if (check_nnodes != 0 || check_tour_length != tour_length)
    {
        ERROR_COMMENT("assert_path", "assert_path failed: check_nnodes=%d, check_tour_length=%lf, nnodes=%d, tour_length=%lf", check_nnodes, check_tour_length, nnodes, tour_length);
        log_path(path, nnodes);
        return 0;
    }
    DEBUG_COMMENT("assert_path", "assert_path passed: check_nnodes=%d, check_tour_length=%lf, nnodes=%d, tour_length=%lf", check_nnodes, check_tour_length, nnodes, tour_length);
    return 1;
}

void generate_random_starting_nodes(int *starting_nodes, int num_nodes, int num_instances, int seed)
{
    srand(seed);
    for (int i = 0; i < num_instances; i++)
    {
        int node = rand() % num_nodes;
        DEBUG_COMMENT("generate_random_starting_nodes", "node=%d", node);
        starting_nodes[i] = node;
    }
}