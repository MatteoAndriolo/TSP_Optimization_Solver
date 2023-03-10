#ifndef GREEDY_H
#define GREEDY_H
#include "vrp.h"
#include "logger.h"

#include <stdlib.h>

/* Calculates the Euclidean distance between two points */
double distance(double point1_x, double point1_y, double point_x, double point_y);

/*Model for the nearst neighboor*/
void model_nearest_neighboor(Instance *inst);

/**/
void generate_distance_matrix(double ** matrix, Instance* inst);

/*simple swap function */
void swap(int** arr, int i, int j);
#endif /* NEAREST_NEIGHBOR_H */
