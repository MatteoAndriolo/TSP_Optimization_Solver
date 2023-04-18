#include "tspcplex.h"
#define EPS 1e-5

void build_model(Instance *inst, CPXENVptr env, CPXLPptr lp)
{    
	INFO_COMMENT("tspcplex.c:build_model", "Building model");
	//double zero = 0.0;  
	char binary = 'B'; 

	char **cname = (char **) calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = (char *) calloc(100, sizeof(char));

// add binary var.s x(i,j) for i < j  

	for ( int i = 0; i < inst->nnodes; i++ )
	{
		for ( int j = i+1; j < inst->nnodes; j++ )
		{
			sprintf(cname[0], "x(%d,%d)", i+1, j+1);		// (i+1,j+1) because CPLEX starts from 1 (not 0
			double obj = dist(i,j,inst); // cost == distance   
			double lb = 0.0;
			double ub = 1.0;
			// eviroment , linear programming, cost of distance of x(i,j), lower bound, upper bound, binary, name of x(i,j)
			if ( CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname) ) print_error(" wrong CPXnewcols on x var.s");
    		if ( CPXgetnumcols(env,lp)-1 != xpos(i,j, inst) ) print_error(" wrong position for x var.s");
		}
	}
	INFO_COMMENT("tspcplex.c:build_model", "number of columns in CPLEX %d", CPXgetnumcols(env,lp)); 

	add_degree_constraint(inst,env, lp);		// add degree constraints (for each node

	free(cname[0]);
	free(cname);
}

void TSPopt(Instance *inst, int *path)
{  
	INFO_COMMENT("tspcplex.c:TSPopt", "Solving TSP with CPLEX");
	// open CPLEX model
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);
	if ( error ) print_error("CPXopenCPLEX() error");
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1"); 
	if ( error ) print_error("CPXcreateprob() error");

	build_model(inst, env, lp);
	INFO_COMMENT("tspcplex.c:TSPopt", "Model built FINISHED");
	
	// Cplex's parameter setting
	CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 123456);	
	CPXsetdblparam(env, CPX_PARAM_TILIM, 3600.0); 
	
	//add_subtour_constraints(inst,env, lp);	
	// ...
	error = CPXmipopt(env,lp);
	if ( error ) 
	{
		printf("CPX error code %d\n", error);
		print_error("CPXmipopt() error"); 
	}

		// add subtour elimination constraints

	// use the optimal solution found by CPLEX
	int *copy_path = (int *) calloc(inst->nnodes * 2, sizeof(int));
	int ncols = CPXgetnumcols(env, lp);
	double *xstar = (double *) calloc(ncols, sizeof(double));
	if ( CPXgetx(env, lp, xstar, 0, ncols-1) ) print_error("CPXgetx() error");
	int count = 0;
	for ( int i = 0; i < inst->nnodes; i++ )
	{
		for ( int j = i+1; j < inst->nnodes; j++ )
		{
			if ( xstar[xpos(i,j,inst)] > 0.5 ){
				DEBUG_COMMENT("tspcplex.c:TSPopt", "x(%d,%d) = %f", i+1, j+1, xstar[xpos(i,j,inst)]);
				copy_path[count] = i + 1;
				copy_path[count + 1] = j + 1;
				count += 2;
			}
		}
	}
	path[0] = copy_path[0];
	path[1] = copy_path[1];
	copy_path[0] = -1;
	copy_path[1] = -1;
	for ( int i = 2; i < inst->nnodes; i++ ) // for loop to write path 
	{
		for (int j = 0; j < inst->nnodes * 2; j++){
			if (path[i-1] == copy_path[j] && j % 2 == 0){
				path[i] = copy_path[j+1];
				copy_path[j] = -1;
				copy_path[j + 1] = -1;
				break;
			}else if (path[i-1] == copy_path[j] && j % 2 == 1){
				path[i] = copy_path[j-1];
				copy_path[j] = -1;
				copy_path[j - 1] = -1;
				break;
			}
		
	}
	DEBUG_COMMENT("tspcplex.c:TSPopt", "path[%d] = %d", i, path[i]);
	}
	

	free(xstar);
	
	// free and close cplex model   
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
}

void build_sol(const double *xstar, Instance *inst, int * succ, int *comp, int* ncomp)
{   

	*ncomp = 0;
	for ( int i = 0; i < inst->nnodes; i++ )
	{
		succ[i] = -1;
		comp[i] = -1;
	}
	
	for ( int start = 0; start < inst->nnodes; start++ )
	{
		if ( comp[start] >= 0 ) continue;  // node "start" was already visited, just skip it

		// a new component is found
		(ncomp)++;
		int i = start;
		int done = 0;
		while ( !done )  // go and visit the current component
		{
			comp[i] = *ncomp;
			done = 1;
			for ( int j = 0; j < inst->nnodes; j++ )
			{
				if ( i != j && xstar[xpos(i,j,inst)] > 0.5 && comp[j] == -1 ) // the edge [i,j] is selected in xstar and j was not visited before 
				{
					succ[i] = j;
					i = j;
					done = 0;
					break;
				}
			}
		}	
		succ[i] = start;  // last arc to close the cycle
			// go to the next component...
	}
}

