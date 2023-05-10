Padua, 11-may-2022

Generic callbacks in Cplex
---------------------------

...
	// install a "lazyconstraint" callback to cut infeasible integer sol.s (found e.g. by heuristics) 
	CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE; // ... means lazyconstraints
	if ( CPXcallbacksetfunc(env, lp, contextid, my_callback, inst) ) print_error("CPXcallbacksetfunc() error");
	
	//CPXsetintparam(env, CPX_PARAM_THREADS, 1); 	// just for debugging

...
	CPXmipopt(env,lp); 		// with the callback installed
	//CPXgetx(...);			// optimal TSP sol. (or just feasible if time limit...), if any
...	


/********************************************************************************************************/
static int CPXPUBLIC my_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle ) 
/********************************************************************************************************/
{ 
	instance* inst = (instance*) userhandle;  
	double* xstar = (double*) malloc(inst->ncols * sizeof(double));  
	double objval = CPX_INFBOUND; 
	if ( CPXcallbackgetcandidatepoint(context, xstar, 0, inst->ncols-1, &objval) ) print_error("CPXcallbackgetcandidatepoint error");
	
	// get some random information at the node (as an example for the students)
	int mythread = -1; CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread); 
	int mynode = -1; CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode); 
	double incumbent = CPX_INFBOUND; CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent); 
	//if ( VERBOSE >= 100 ) printf(" ... callback at node %5d thread %2d incumbent %10.2lf, candidate value %10.2lf\n", .....);
	
	...

	int nnz = 0; 
	... if xstart is infeasible, find a violated cut and store it in the usual Cplex s data structute (rhs, sense, nnz, index and value)
	
	if ( nnz > 0 ) // means that the solution is infeasible and a violated cut has been found
	{	
		int izero = 0;		
		if ( CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &izero, index, value) ) print_error("CPXcallbackrejectcandidate() error"); // reject the solution and adds one cut 

		//if ( CPXcallbackrejectcandidate(context, 0, NULL, NULL, NULL, NULL, NULL, NULL) ) print_error("CPXcallbackrejectcandidate() error"); // just reject the solution without adding cuts (less effective)
	}
	
	free(xstar); 
	return 0; 
}


Both lazy contraints and usercuts
----------------------------------

...
	CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_RELAXATION; // both lazy and usercuts
	if ( CPXcallbacksetfunc(env, lp, contextid, my_callback, inst) ) print_error("CPXcallbacksetfunc() error");
...	

	CPXmipopt(env,lp); 		// with the callback installed
...	


/********************************************************************************************************/
static int CPXPUBLIC my_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle ) 
/********************************************************************************************************/
{ 
	instance* inst = (instance*) userhandle;  

	double* xstar = (double*) malloc(inst->ncols * sizeof(double));   
	double objval = CPX_INFBOUND; 
	if ( contextid == CPX_CALLBACKCONTEXT_CANDIDATE && CPXcallbackgetcandidatepoint(context, xstar, 0, inst->ncols-1, &objval) ) print_error("CPXcallbackgetcandidatepoint error");
	if ( contextid == CPX_CALLBACKCONTEXT_RELAXATION && CPXcallbackgetrelaxationpoint(context, xstar, 0, inst->ncols-1, &objval) ) print_error("CPXcallbackgetrelaxationpoint error");

	if ( ... )
	{
		...
		
		int izero = 0;
		int purgeable = CPX_USECUT_FILTER;
		int local = 0;

		if ( contextid == CPX_CALLBACKCONTEXT_CANDIDATE && CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &izero, cutind, cutval) ) print_error("CPXcallbackrejectcandidate() error"); // reject the solution and add one cut 
					
		if ( contextid == CPX_CALLBACKCONTEXT_RELAXATION && CPXcallbackaddusercuts(context, 1, nnz, &rhs, &sense, &izero, cutind, cutval,&purgeable, &local) ) print_error("CPXcallbackaddusercuts() error"); // add one violated usercut 
	}
	
	...
	
	free(xstar);
	return 0; 
}


Alternatively, you can use a simple driver:
---------------------------------------------

/********************************************************************************************************/
static int CPXPUBLIC my_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle ) 
/********************************************************************************************************/
{ 
	instance* inst = (instance*) userhandle;  
	if ( contextid == CPX_CALLBACKCONTEXT_CANDIDATE ) return my_callback_candidate(context, inst); 
	if ( contextid == CPX_CALLBACKCONTEXT_RELAXATION ) return my_callback_relaxation(context, inst);
	print_error("contextid unknownn in my_callback"); 
	return 1; 	
}

-----------------------
USEFUL PIECES OF CODE
-----------------------

1) Converting a "successor" TSP solution succ[0..nnodes-1] to a CPLEX solution xheu[0..ncols-1]
---------------------------------------------------------------------------------------------------

	double *xheu = (double *) calloc(inst->ncols, sizeof(double));  // all zeros, initially
	for ( int i = 0; i < inst->nnodes; i++ ) xheu[xpos(i,succ[i],inst)] = 1.0;
	...
	free(xheu);
	
	

2) Adding a mipstart solution xheu[0..ncols-1] before calling CPXmipopt (i.e., not from a callback)
-----------------------------------------------------------------------------------------------------

	int *ind = (int *) malloc(inst->ncols * sizeof(int));
	for ( int j = 0; j < inst->ncols; j++ ) ind[j] = j;
	int effortlevel = CPX_MIPSTART_NOCHECK;  
	int beg = 0; 						
	if ( CPXaddmipstarts(env, lp, 1, inst->ncols, &beg, indices, xheu, &effortlevel, NULL) print_error("CPXaddmipstarts() error");	
	free(ind);
	
	
3) Posting a heuristic TSP solution xheu[0..ncols-1] within the CPX_CALLBACKCONTEXT_CANDIDATE callback
---------------------------------------------------------------------------------------------------------

	... starting from the integer xstar, use the patching heuristic + 2-opt to build a heuristic TSP solution xheu[0..ncols-1] of value objheu, and "post it" so CPLEX can use it


	int *ind = (int *) malloc(inst->ncols * sizeof(int));
	for ( int j = 0; j < inst->ncols; j++ ) ind[j] = j;
	if ( CPXcallbackpostheursoln(context, inst->ncols, ind, xheu, objheu, CPXCALLBACKSOLUTION_NOCHECK) ) print_error("CPXcallbackpostheursoln() error");
	free(ind);
