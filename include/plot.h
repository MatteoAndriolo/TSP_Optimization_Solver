#ifndef PLOT_H
#define PLOT_H
#include "../include/vrp.h"

/**
 * Plot the path
 *
 * @param path the path to plot
 * @param x the x coordinates of the nodes
 * @param y the y coordinates of the nodes
 * @param nnodes the number of nodes
 * @param title the title of the plot
 * @param starting_node the starting node
 */
void plot(const int *path, const double *x, const double *y, const int nnodes,
          const char *title, const int starting_node);

#endif