#ifndef VRP_H_
#define VRP_H_

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <cplex.h>

// #include <cplex.h>
#include <pthread.h>

// hard-wired parameters
#define XSMALL 1e-5				// 1e-4*	// tolerance used to decide ingerality of 0-1 var.s
#define EPSILON 1e-9			// 1e-9		// very small numerical tolerance
#define TICKS_PER_SECOND 1000.0 // cplex's ticks on Intel Core i7 quadcore @2.3GHZ
#define INFTY 1e+30

typedef struct
{
	// input data
	int nnodes;
	double *x;
	double *y;

	// MODEL
	int model_type;
	int integer_costs;

	double zbest; // best sol. available
	double tour_lenght;
	int *path_best;

	int node_start;
	// int xstart;
	// int ystart;

	// int qstart;
	// int bigqstart;
	// int sstart;
	// int bigsstart;i
	// int fstart;
	// int zstart;

	// execution parameters
	int randomseed;
	double timelimit;	   // overall time limit, in sec.s
	char input_file[1000]; // input file
	char log_file[1000];   // output log file
	// int num_threads;
	//  int verbosity;							// verbosity (1: incumbement, 5:debug)
	// char node_file[1000];  // cplex node file

	// parameters
	// int available_memory;
	// int max_nodes; // max n. of branching nodes in the final run (-1 unlimited)
	// double cutoff; // cutoff (upper bound) for master

	// grasp

	// global data
	int ncols;
	double tstart; // starting time
				   // double tbest;	  // time for the best sol. available
				   // double best_lb;	  // best lower bound available
				   // double *load_min; // minimum load when leaving a node
				   // double *load_max; // maximum load when leaving a node

} Instance;

// inline
inline int imax(int i1, int i2) { return (i1 > i2) ? i1 : i2; }
inline double dmin(double d1, double d2) { return (d1 < d2) ? d1 : d2; }
inline double dmax(double d1, double d2) { return (d1 > d2) ? d1 : d2; }
#endif /* VRP_H_ */
