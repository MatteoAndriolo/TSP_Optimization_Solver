1) How to install a lazyconstraint callback?

...
	CPXsetintparam(env, CPX_PARAM_MIPCBREDLP, CPX_OFF);	// let MIP callbacks work on the original model
	CPXsetlazyconstraintcallbackfunc(env, mylazycallback, inst);
	int ncores = 1; CPXgetnumcores(env, &ncores);
	CPXsetintparam(env, CPX_PARAM_THREADS, ncores); 	// it was reset after callback

...
	inst->ncols = CPXgetnumcols(env,lp);
...

	CPXmipopt(env,lp); 		// with the callback installed

2) Example code for  mylazycallback()

void print_error(const char *err) { printf("\n\n ERROR: %s \n\n", err); exit(1); } 

/**************************************************************************************************************************/
static int CPXPUBLIC mylazycallback(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p)
/**************************************************************************************************************************/
{
		*useraction_p = CPX_CALLBACK_DEFAULT;
		instance* inst = (instance *) cbhandle; 			// casting of cbhandle

		// get solution xstar
		double *xstar = (double*) malloc(inst->ncols * sizeof(double));
		if ( CPXgetcallbacknodex(env, cbdata, wherefrom, xstar, 0, inst->ncols-1) ) return 1; // xstar = current x from CPLEX-- xstar starts from position 0

		// get some random information at the node (as an example)
		double objval = CPX_INFBOUND; CPXgetcallbacknodeobjval(env, cbdata, wherefrom, &objval);
		int mythread = -1; CPXgetcallbackinfo(env, cbdata, wherefrom, CPX_CALLBACK_INFO_MY_THREAD_NUM, &mythread);
		double zbest; CPXgetcallbackinfo(env, cbdata, wherefrom, CPX_CALLBACK_INFO_BEST_INTEGER, &zbest);

		//apply cut separator and possibly add violated cuts
		int ncuts = myseparation(inst, xstar, env, cbdata, wherefrom);
		free(xstar);

		if ( ncuts >= 1 ) *useraction_p = CPX_CALLBACK_SET; 		// tell CPLEX that cuts have been created
		return 0; 													// return 1 would mean error --> abort Cplex's execution
}


3) How to add a cut within myseparation()?

...

	if ( CPXcutcallbackadd(env, cbdata, wherefrom, nnz, rhs, sense, index, value, 0) ) print_error("USER_separation: CPXcutcallbackadd error");
	//if ( CPXcutcallbackaddlocal(env, cbdata, wherefrom, nnz, rhs, sense, index, value) ) print_error("USER_separation: CPXcutcallbackaddlocal error");

...

4) Generic callback: this is a unified callback with a number of advantages w.r.t. legacy ones (notably: dynamic search is not disabled). Here is an example of use 


...
	// installing a generic callback (CPX_CALLBACKCONTEXT_CANDIDATE means lazy cuts)
	CPXsetintparam(env, CPX_PARAM_MIPCBREDLP, CPX_OFF);	// let MIP callbacks work on the original model
	if ( CPXcallbacksetfunc(env, lp, CPX_CALLBACKCONTEXT_CANDIDATE, my_callback, inst) ) print_error("CPXcallbacksetfunc() error");
	//int ncores = 1; CPXgetnumcores(env, &ncores); CPXsetintparam(env, CPX_PARAM_THREADS, ncores); // no longer reset after installing callback 
...
	if ( CPXmipopt(env,lp) ) print_error("CPXmipopt error 1");
...	

/********************************************************************************************************/
static int CPXPUBLIC my_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle ) 
/********************************************************************************************************/
{ 
	instance* inst = (instance*) userhandle;  
	double* xstar = (double*) malloc(inst->ncols * sizeof(double));  
	double objval = CPX_INFBOUND; 
	if ( CPXcallbackgetcandidatepoint(context, xstar, 0, inst->ncols-1, &objval) ) print_error("CPXcallbackgetcandidatepoint error");
	
	// get some information at the node (as an example)
	int mythread = -1; CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread); 
	double zbest; CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &zbest); 
	printf("  my_callback[%d]... obj %lf zbest %lf\n", mythread, objval, zbest);
	
	//if ( myseparation(inst, xstar, context)) print_error("my_separation() error");
	
			
	//if ( CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &izero, index, value) ) print_error("CPXcallbackrejectcandidate() error"); // reject the solution and adds one cut 

	//if ( CPXcallbackrejectcandidate(context, 0, NULL, NULL, NULL, NULL, NULL, NULL) ) print_error("CPXcallbackrejectcandidate() error"); // just reject the solution without adding cuts 
	
	
	free(xstar); 
	return 0; 
}


