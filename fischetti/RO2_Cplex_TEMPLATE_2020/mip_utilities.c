#include <cplex.h>
#include "vrp.h" 

typedef struct {
	int *ptr_giveup;   
	instance *inst;
	int mythread; 
	double timelimit;      
	double zlp;
	double *internal_point;
	CPXENVptr env;
	CPXLPptr lp;  
} bundle;        


int my_separation(int ncols, double *xstar, int purgeable, CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle);   
int pool_load_cuts(CPXENVptr env, CPXLPptr lp, instance *inst);
void ellipsoid_for_dummies(CPXENVptr env, CPXLPptr lp, instance *inst, int num_random);
int mip_remove_slack_cuts(CPXENVptr env, CPXLPptr lp, int nrows_keep);     
double get_solution_pool(instance *inst, int mythread, double incumbent, double *x);
void create_blurred_local_instance(instance *inst, double *ystar);
void xprint(int n, const double *x, int verbose);
double random01(); 
void double_vector_copy(int n, const double *from, double *to);

double second();
void print_error(const char *err);
int num_cores(instance *inst);       

double mip_sol_distance(int n, double *x1, double *x2)
{
	double dist = 0.0;
	for ( int j = 0; j < n; j++ ) dist += (x1[j]-x2[j])*(x1[j]-x2[j]);
	return sqrt(dist);
}

double mip_value(CPXENVptr env, CPXLPptr lp)
{
	double zz;
	if ( CPXgetobjval(env, lp, &zz) ) zz = CPX_INFBOUND;
	return zz;
}

void mip_delete_all_mipstarts(CPXENVptr env, CPXLPptr lp)
{
	//return;
	int nmipstart = CPXgetnummipstarts(env, lp);
	if ( nmipstart > 0 && CPXdelmipstarts (env, lp, 0, nmipstart-1) ) print_error("CPXdelmipstarts error");
}

void mip_timelimit(CPXENVptr env, double timelimit, instance *inst)
{
	double residual_time = inst->tstart + inst->timelimit - second();
	if ( residual_time < 0.0 ) residual_time = 0.0;
	CPXsetintparam(env, CPX_PARAM_CLOCKTYPE, 2);
	CPXsetdblparam(env, CPX_PARAM_TILIM, residual_time); 							// real time
	CPXsetdblparam(env, CPX_PARAM_DETTILIM, TICKS_PER_SECOND*timelimit);			// ticks
}

int mip_solution_available(CPXENVptr env, CPXLPptr lp)
{
	double zz;
	if ( CPXgetobjval(env, lp, &zz) ) return 0;
	return 1;
}


int mip_solved_to_optimality(CPXENVptr env, CPXLPptr lp)
{
	int lpstat = CPXgetstat (env, lp);
	if ( VERBOSE >= 100 ) printf(" CPLEX lpstat %d\n", lpstat);
	int solved = ( lpstat == CPXMIP_OPTIMAL ) ||
				 ( lpstat == CPXMIP_OPTIMAL_INFEAS ) ||
				 //( lpstat ==  CPXMIP_OPTIMAL_RELAXED ) ||
				 ( lpstat ==  CPXMIP_OPTIMAL_TOL );
	return solved;
}

int mip_infeasible(CPXENVptr env, CPXLPptr lp)
{
	int lpstat = CPXgetstat(env, lp);
	if ( VERBOSE >= 100 ) printf(" CPLEX lpstat %d\n", lpstat);
	int infeas = ( lpstat == CPXMIP_INFEASIBLE );
	return infeas;
}



void mip_set_level_for_all_cuts(CPXENVptr env, int level)
{
	CPXsetintparam(env, CPX_PARAM_CLIQUES         , level);
	CPXsetintparam(env, CPX_PARAM_COVERS          , level);
	CPXsetintparam(env, CPX_PARAM_DISJCUTS        , level);
	CPXsetintparam(env, CPX_PARAM_FLOWCOVERS      , level);
	CPXsetintparam(env, CPX_PARAM_FLOWPATHS       , level);
	CPXsetintparam(env, CPX_PARAM_FRACCUTS        , level);
	CPXsetintparam(env, CPX_PARAM_GUBCOVERS       , level);
	CPXsetintparam(env, CPX_PARAM_IMPLBD          , level);
	CPXsetintparam(env, CPX_PARAM_MIRCUTS         , level);
	CPXsetintparam(env, CPX_PARAM_ZEROHALFCUTS    , level);
	CPXsetintparam(env, CPX_PARAM_LANDPCUTS       , level);
	CPXsetintparam(env, CPX_PARAM_MCFCUTS         , level);
}

void mip_restart(CPXENVptr env, CPXLPptr lp)
{
// I guess there is a cleaner way...
	double clb;
	double cub;
	int zero = 0;
	CPXgetlb(env, lp, &clb, 0, 0);
	CPXgetub(env, lp, &cub, 0, 0);
	char low = 'L';
	double fakelb = cub + 1.0;
	CPXchgbds(env, lp, 1, &zero, &low, &fakelb);
	mip_delete_all_mipstarts(env, lp);
	CPXchgbds(env, lp, 1, &zero, &low, &clb);
}

int mip_solution_hash(CPXENVptr env, CPXLPptr lp)
{
	if ( !mip_solution_available(env, lp) ) return -1;

	int ncols = CPXgetnumcols(env, lp);
	double *xstar = (double *) calloc(ncols, sizeof(double));
	char *ctype = (char *) calloc(ncols, sizeof(char));
	CPXgetx(env, lp, xstar, 0, ncols-1);
	CPXgetctype(env, lp, ctype, 0, ncols-1);

	const int max_hash = 123456789;
	int hash = 0;
	for ( int i = 0; i < ncols; i++ )
	{
		if ( ctype[i] == 'B' && xstar[i] > 0.5 ) hash = (hash + i*i + 17*i) % max_hash;
	}
	free(ctype);
	free(xstar);
	return hash;
}

int mip_update_incumbent(CPXENVptr env, CPXLPptr lp, instance *inst)
{
	int ncols = CPXgetnumcols(env, lp);

	int newsol = 0;

	if ( mip_value(env,lp) < inst->zbest - EPSILON )
	{
		inst->tbest = second() - inst->tstart;
		inst->zbest = mip_value(env, lp);
		CPXgetx(env, lp, inst->best_sol, 0, ncols-1);
		if ( VERBOSE >= 40 ) printf("\n >>>>>>>>>> incumbent update of value %lf at time %7.2lf <<<<<<<<\n", inst->zbest, inst->tbest);
		newsol = 1;
	}     
	
	// save the solution in a file (does not work if the callbacks changed it...)
	if ( newsol && (VERBOSE >= 10) )
	{
		if ( VERBOSE >= 100 ) CPXwritemipstarts(env, lp, "model.mst", 0, 0);
		printf("... New incumbent of value %20.5lf (hash %12d) found after %7.2lf sec.s \n", inst->zbest, mip_solution_hash(env, lp), inst->tbest);
		fflush(NULL);
	}

	return newsol;
}


void mip_change_bounds(CPXENVptr env, CPXLPptr lp, int jvar, double clb, double cub)
{
  char low = 'L', upp = 'U';
  CPXchgbds(env, lp, 1, &jvar, &low, &clb);
  CPXchgbds(env, lp, 1, &jvar, &upp, &cub);
}

int mip_change_rhs(CPXENVptr env, CPXLPptr lp, int irow, const double rhs)
{
	return CPXchgrhs(env, lp, 1, &irow, &rhs); 
}

int num_cores(instance *inst)
{                          
	int ncores = inst->num_threads;
	if ( ncores == 0 ) 
	{
		int error;
		CPXENVptr env = CPXopenCPLEX(&error);
		CPXgetnumcores(env, &ncores);  
		CPXcloseCPLEX(&env);  
	}
	return ncores;
}



/**********************************************************************************************************/
int mip_remove_slack_cuts(CPXENVptr env, CPXLPptr lp, int nrows_keep)
/**********************************************************************************************************/
{   
	
	int nrows = CPXgetnumrows(env,lp);
	//printf("skipping mip_remove_slack_cuts...\n"); return nrows;

	int *delstat = (int *) calloc(nrows, sizeof(int));
	double *slack = (double *) calloc(nrows, sizeof(double));
	double *rhs = (double *) calloc(nrows, sizeof(double));

	if ( CPXgetslack(env, lp, slack, 0, nrows-1) ) print_error(" mip_remove_slack_cuts(): no slack solution from the final LP??");
	if ( CPXgetrhs(env, lp, rhs, 0, nrows-1) ) print_error(" mip_remove_slack_cuts(): no rhs??");

	for ( int i = nrows_keep; i < nrows; i++ ) delstat[i] = ( fabs(slack[i]) > 100*XSMALL+0.0001*fabs(rhs[i]) );

	if ( CPXdelsetrows (env, lp, delstat) )  print_error(" mip_remove_slack_cuts(): CPXdelsetrows error");

	free(rhs);
	free(slack);
	free(delstat);

	if ( VERBOSE >= 100 ) printf(" >>>>>>>>> mip_remove_slack_cuts() removed %d cuts\n", nrows-CPXgetnumrows(env,lp));

	return nrows-CPXgetnumrows(env,lp);
}  


/**********************************************************************************************************/
double mip_getobj(CPXENVptr env, CPXLPptr lp, const double *x)
/**********************************************************************************************************/
{
	int ncols = CPXgetnumcols(env, lp);
	double *obj = (double *) calloc(ncols, sizeof(double));
	CPXgetobj(env, lp, obj, 0, ncols-1);
	double value = 0.0;
	for ( int j = 0; j < ncols; j++ ) value += obj[j] * x[j];
	free(obj);
	return value;
}


/**********************************************************************************************************/
void _old_mip_reload_solution(CPXENVptr env, CPXLPptr lp, int ncols, double *xstar)
/**********************************************************************************************************/
{

//reload solution xstar[0..ncols-1]

	int sollim; CPXgetintparam(env, CPX_PARAM_INTSOLLIM, &sollim);
	int screen; CPXgetintparam(env, CPX_PARAM_SCRIND, &screen);
	int nnodes; CPXgetintparam(env, CPX_PARAM_NODELIM, &nnodes);
	int advind; CPXgetintparam(env, CPX_PARAM_ADVIND, &advind);
	int heufreq; CPXgetintparam(env, CPX_PARAM_HEURFREQ, &heufreq);
	double cutup; CPXgetdblparam(env, CPX_PARAM_CUTUP, &cutup);

	// cleanup binary var.s in xstar
	char *ctype = (char *) calloc(ncols, sizeof(char));
	CPXgetctype(env, lp, ctype, 0, ncols-1);
	int j;
	for ( j = 0; j < ncols; j++ )
	{
		if ( ctype[j] == 'B' ) xstar[j] =  (xstar[j] < 0.5) ? 0.0 : 1.0;
	}
	free(ctype);

	int *indices = (int *) malloc(ncols*sizeof(int));
	for ( j = 0; j < ncols; j++ ) indices[j] = j;

	mip_delete_all_mipstarts(env, lp);
	int beg = 0;
	int effortlevel = CPX_MIPSTART_CHECKFEAS; 										// it was CPX_MIPSTART_SOLVEMIP;
	CPXaddmipstarts(env, lp, 1, ncols, &beg, indices, xstar, &effortlevel, NULL); 	// it was CPXcopymipstart(env, lp, ncols, indices, xstar);
	free(indices);

	CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	CPXsetintparam(env, CPX_PARAM_INTSOLLIM, 1);
	//CPXsetintparam(env, CPX_PARAM_ADVIND, 2);
	//CPXsetintparam(env, CPX_PARAM_NODELIM, 0);  			// it is no longer correct for cplex 12.6.1+
	CPXsetintparam(env, CPX_PARAM_NODELIM, 10); 			//  0 for 12.6-

	double myval = mip_getobj(env, lp, xstar);
	if ( VERBOSE >= 1000 ) printf(" \n\n\n\n\n ################ mip_reload_solution(): input solution has value %lf\n", myval);
	CPXsetdblparam(env, CPX_PARAM_CUTUP, myval+0.001*fabs(myval));		   // required for cplex 12.6.1 ??

	CPXmipopt(env, lp);

	CPXsetintparam(env, CPX_PARAM_INTSOLLIM, sollim);
	CPXsetintparam(env, CPX_PARAM_SCRIND, screen);
	CPXsetintparam(env, CPX_PARAM_NODELIM, nnodes);
	CPXsetintparam(env, CPX_PARAM_ADVIND, advind);
	CPXsetintparam(env, CPX_PARAM_HEURFREQ, heufreq);
	CPXsetdblparam(env, CPX_PARAM_CUTUP, cutup);

	double zz;
	if ( CPXgetobjval(env, lp, &zz) )
	{
		printf(" mip_reload_solution(): no solution after reloading??\n\n");
		//exit(1);
	}

	if ( VERBOSE >= 1000 ) printf(" ################ solution of value %lf (%lf) reloaded!!!!!!!!!!!!!!!!!!!!\n\n", mip_value(env,lp),myval);

} 

/**********************************************************************************************************/
static int CPXPUBLIC relaod_heurcallbackfunct(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, double *objval_p, double *x, int *checkfeas_p,int *useraction_p)
/**********************************************************************************************************/
{   
	*useraction_p = CPX_CALLBACK_DEFAULT;
	double *sol = (double *) cbhandle;    
	double zbest; CPXgetcallbackinfo(env, cbdata, wherefrom, CPX_CALLBACK_INFO_BEST_INTEGER, &zbest); 
	if ( zbest <= (1.0+1e-10) * sol[0] ) return 1;
	int ncols = sol[1] + 0.1;   
	for ( int j = 0; j < ncols; j++ ) x[j] = sol[j+2];
	*objval_p = sol[0];    
	*checkfeas_p = 0;
	*useraction_p = CPX_CALLBACK_SET;    
	return 0;
} 


/**********************************************************************************************************/
void mip_reload_solution(CPXENVptr env, CPXLPptr lp, int ncols, double *xstar)     
/**********************************************************************************************************/
{

//reload solution xstar[0..ncols-1] without using mipstarts that are handled differently by different Cplex versions

	double t1 = second();
	int ncols_lp = CPXgetnumcols(env,lp);				  
	if ( ncols > ncols_lp ) { printf(" mip_reload_solution(): ncols error %d > %d\n", ncols, ncols_lp); exit(30); }
	
	double *sol = (double *) calloc(ncols_lp+2, sizeof(double));    

	// store the solution to reload, with sol[0] = solution value, sol[1] = ncols, sol[2...] = x[0...]
	double *obj = (double *) calloc(ncols, sizeof(double));
	CPXgetobj(env, lp, obj, 0, ncols-1);
	double value = 0.0;
	for ( int j = 0; j < ncols; j++ ) 
	{
		value += obj[j] * xstar[j]; 
		sol[j+2] = xstar[j];         			// shifted
	}
	free(obj); 
	sol[0] = value;   
	sol[1] = ncols;
	if ( VERBOSE >= 1000 ) printf(" \n\n\n\n\n ################ mip_reload_solution(): input solution has value %lf\n", value);
	
    // install callbacks
	void *cbhandle_heur; 
	int (*cbheurfunct_p) (CPXCENVptr, void *, int, void *, double *, double *, int *, int *); 
	if ( CPXgetheuristiccallbackfunc(env, &cbheurfunct_p, &cbhandle_heur) ) { printf(" mip_reload_solution(): CPXgetheuristiccallbackfunc??\n"); exit(31); }
	if ( CPXsetheuristiccallbackfunc(env, relaod_heurcallbackfunct, sol) ) { printf(" mip_reload_solution(): CPXsetheuristiccallbackfunc??\n"); exit(32); }

	//also add mipstarts (just in case)
	int *indices = (int *) malloc(ncols*sizeof(int));
	for ( int j = 0; j < ncols; j++ ) indices[j] = j;
	int nmipstart = CPXgetnummipstarts(env, lp);
	if ( nmipstart > 0 && CPXdelmipstarts (env, lp, 0, nmipstart-1) ){ printf(" mip_reload_solution(): CPXdelmipstarts??\n"); exit(11); } 
	int beg = 0;
	int effortlevel = CPX_MIPSTART_CHECKFEAS; 										// it was CPX_MIPSTART_SOLVEMIP;
	CPXaddmipstarts(env, lp, 1, ncols, &beg, indices, xstar, &effortlevel, NULL); 	// it was CPXcopymipstart(env, lp, ncols, indices, xstar);
	free(indices);

	// solve and return asap   
	int screen; CPXgetintparam(env, CPX_PARAM_SCRIND, &screen);
	int nnodes; CPXgetintparam(env, CPX_PARAM_NODELIM, &nnodes);
	int heufreq; CPXgetintparam(env, CPX_PARAM_HEURFREQ, &heufreq);   
	int preind; CPXgetintparam(env, CPX_PARAM_PREIND, &preind);   
	CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	CPXsetintparam(env, CPX_PARAM_NODELIM, 0);					//  to be safe
	CPXsetintparam(env, CPX_PARAM_HEURFREQ, -1); 				// ??
	CPXsetintparam(env, CPX_PARAM_PREIND, 0); 
	CPXmipopt(env, lp);
	CPXsetintparam(env, CPX_PARAM_SCRIND, screen);
	CPXsetintparam(env, CPX_PARAM_NODELIM, nnodes);
	CPXsetintparam(env, CPX_PARAM_HEURFREQ, heufreq);
	CPXsetintparam(env, CPX_PARAM_PREIND, preind);

    // reinstall the origianl callback
	if ( CPXsetheuristiccallbackfunc(env, cbheurfunct_p, cbhandle_heur) ) { printf(" mip_reload_solution(): final CPXsetheuristiccallbackfunc??\n"); exit(33); }

	double zz; if ( CPXgetobjval(env, lp, &zz) ) printf(" mip_reload_solution(): no solution after reloading??????\n");
	if ( VERBOSE >= 100 ) printf(" ########### solution of value %lf (expected %lf) reloaded in %.5lf sec.s ######################\n\n", zz, value, second()-t1);  
	
	free(sol);
}

/**********************************************************************************************************/
void mip_add_cutoff_constraint(CPXENVptr env, CPXLPptr lp, double cutoff)
/**********************************************************************************************************/
{
	int ncols = CPXgetnumcols(env, lp);

	// get the  objective function
	double *true_obj = (double *) calloc(ncols, sizeof(double));
	CPXgetobj(env, lp, true_obj, 0, ncols-1);

	int *indices = (int *) calloc(ncols, sizeof(int));
	double *values = (double *) calloc(ncols, sizeof(double));
	int nnz = 0;
	for ( int j = 0; j < ncols; j++ )
	{
		if ( fabs(true_obj[j]) > EPSILON )
		{
			indices[nnz] = j;
			values[nnz] = true_obj[j];
			nnz++;
		}
	}
	char leq = 'L';
	double rhs = cutoff;
	int izero = 0;
	char *rowname = (char *) calloc(20, sizeof(char));
	sprintf(rowname, "_CUTOFF_");
	CPXaddrows(env, lp, 0, 1, nnz, &rhs, &leq, &izero, indices, values, NULL, &rowname);
	free(rowname);
	free(values);
	free(indices);
	free(true_obj);
}


/*********************************************************************************************************************************************************/
void mip_add_cut(void *env, void *cbdata, int wherefrom, int nnz, double rhs, char sense, int *index, double *value, int purgeable)
/*********************************************************************************************************************************************************/
{

/*
	unified driver to add constraints to the LP; usage:

		mip_add_cut((void *) env, (void *) lp, -1, nnz, rhs, sense, index, value, -1);   			// -1 means static cuts
		mip_add_cut((void *) env, (void *) lp, -1, nnz, rhs, sense, index, value, -2);   			// -2 means lazy cuts
		mip_add_cut((void *) env, (void *) lp, -1, nnz, rhs, sense, index, value, -3);   			// -3 means user cuts
		mip_add_cut((void *) env, cbdata, wherefrom, nnz, rhs, sense, index, value, purgeable); 	// from callbacks, user cut
		mip_add_cut((void *) env, cbdata, wherefrom, nnz, rhs, sense, index, value, -4); 			// from callbacks, local cut
*/

	if ( purgeable >= 0 ) 					// add the cut from a callback
	{
		if ( CPXcutcallbackadd((CPXCENVptr) env, cbdata, wherefrom, nnz, rhs, sense, index, value, purgeable) ) print_error("error 1 in mip_add_cut()");
		return;
	}

	int izero = 0;

	if ( purgeable == -1 ) 					// statically add the cut to the original model (with casting (CPXLPptr) cbdata)
	{
		if ( CPXaddrows( (CPXENVptr) env, (CPXLPptr) cbdata, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, NULL) ) print_error("error 2 in mip_add_cut()");
		return;
	}

	if ( purgeable == -2 ) 					// add the lazycut cut to the original model (with casting (CPXLPptr) cbdata)
	{
		if ( CPXaddlazyconstraints((CPXENVptr) env, (CPXLPptr) cbdata, 1, nnz, &rhs, &sense, &izero, index, value, NULL) ) print_error("error 3 in mip_add_cut()");
		return;
	}

	if ( purgeable == -3 ) 					// add the usercut cut to the original model (with casting (CPXLPptr) cbdata)
	{
	 	if ( CPXaddusercuts((CPXENVptr) env, (CPXLPptr) cbdata, 1, nnz, &rhs, &sense, &izero, index, value, NULL) ) print_error("error 4 in mip_add_cut()");
		return;
	}

	if ( purgeable == -4 ) 					// add the usercut *local* to the original model (with casting (CPXLPptr) cbdata)
	{
		if ( CPXcutcallbackaddlocal((CPXCENVptr) env, cbdata, wherefrom, nnz, rhs, sense, index, value) ) print_error("error local cuts in mip_add_cut()");
		return;
	}

	print_error("purgeable flag not known in mip_add_cut()");
}  












