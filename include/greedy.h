#ifndef GREEDY_H
#define GREEDY_H
#include "vrp.h"
#include "logger.h"
#include <stdlib.h>
#include <math.h>

double distance(double point1_x, double point1_y, double point_x, double point_y);

void extra_mileage(Instance* inst);

void model_nearest_neighboor(Instance *inst);

void generate_distance_matrix(double ** matrix, Instance* inst);

void swap(int** arr, int i, int j);

#endif /* NEAREST_NEIGHBOR_H */
