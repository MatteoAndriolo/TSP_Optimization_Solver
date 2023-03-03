#ifndef VRP_H_  

#define VRP_H_

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h> 
#include <stdio.h>  

#include <cplex.h>  
#include <pthread.h>  

#define VERBOSE				    50		// printing level  (=10 only incumbent, =20 little output, =50-60 good, =70 verbose, >=100 cplex log)

//hard-wired parameters
#define XSMALL		  		  1e-5 		// 1e-4*	// tolerance used to decide ingerality of 0-1 var.s
#define EPSILON		  		  1e-9		// 1e-9		// very small numerical tolerance 
#define TICKS_PER_SECOND 	  1000.0  	// cplex's ticks on Intel Core i7 quadcore @2.3GHZ
                                 
//data structures  

typedef struct {   
	
	//input data
	int nnodes; 	
	double *demand;   
	double *xcoord;
	double *ycoord;
	int depot;
	double capacity; 
	int nveh;

	// parameters 
	int model_type; 
	int old_benders;
	int randomseed;
	int num_threads;
	double timelimit;						// overall time limit, in sec.s
	char input_file[1000];		  			// input file
	char node_file[1000];		  			// cplex node file
	int available_memory;
	int max_nodes; 							// max n. of branching nodes in the final run (-1 unlimited)
	double cutoff; 							// cutoff (upper bound) for master
	int integer_costs;

	//global data
	double	tstart;								
	double zbest;							// best sol. available  
	double tbest;							// time for the best sol. available  
	double *best_sol;						// best sol. available    
	double	best_lb;						// best lower bound available  
	double *load_min;						// minimum load when leaving a node
	double *load_max;						// maximum load when leaving a node
	
	// model;     
	int xstart;
	int qstart;
	int bigqstart;  						
	int sstart;
	int bigsstart;
	int ystart;
	int fstart;
	int zstart;
} instance;        

//inline
inline int imax(int i1, int i2) { return ( i1 > i2 ) ? i1 : i2; } 
inline double dmin(double d1, double d2) { return ( d1 < d2 ) ? d1 : d2; } 
inline double dmax(double d1, double d2) { return ( d1 > d2 ) ? d1 : d2; } 

#endif   /* VRP_H_ */ 
