#ifndef PLOT_H
#define PLOT_H
#include "vrp.h"

/**
 * @brief Plot the solution
 *
 * @param inst
 * @return void
 */
void plot(const int *path, const double *x, const double *y, const int nnodes, const char *title);

#endif