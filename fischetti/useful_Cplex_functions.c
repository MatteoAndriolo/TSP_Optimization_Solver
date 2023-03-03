
#define VERBOSE 10  // printing level

int imax(int i1, int i2) { return ( i1 > i2 ) ? i1 : i2; }
int imin(int i1, int i2) { return ( i1 < i2 ) ? i1 : i2; }
double dmax(double d1, double d2) { return ( d1 > d2 ) ? d1 : d2; }
double dmin(double d1, double d2) { return ( d1 < d2 ) ? d1 : d2; }
void debug(const char *err) { printf("\nDEBUG: %s \n", err); fflush(NULL); }
void print_error(const char *err) { printf("\n\n STAT, ERROR: %s \n\n", err); fflush(NULL); exit(1); }
double random01() { return ((double) rand() / RAND_MAX); } // return a random value in range 0.0-1.0
double randomDbl(double max) { return max * ((double) rand() / RAND_MAX); } // return a random value in range 0.0 - max


/*********************************************************************************************************************************************************/
double mip_value(CPXENVptr env, CPXLPptr lp)
/*********************************************************************************************************************************************************/
{
	double zz;
	if ( CPXgetobjval(env, lp, &zz) ) zz = CPX_INFBOUND;
	return zz;
}

/*********************************************************************************************************************************************************/
void mip_delete_all_mipstarts(CPXENVptr env, CPXLPptr lp)
/*********************************************************************************************************************************************************/
{
	int nmipstart = CPXgetnummipstarts(env, lp);
	if ( nmipstart > 0 && CPXdelmipstarts (env, lp, 0, nmipstart-1) ) print_error("CPXdelmipstarts error");
}


/*********************************************************************************************************************************************************/
int mip_solution_available(CPXENVptr env, CPXLPptr lp)
/*********************************************************************************************************************************************************/
{
	double zz;
	if ( CPXgetobjval(env, lp, &zz) ) return 0;
	return 1;
}


/*********************************************************************************************************************************************************/
int mip_solved_to_optimality(CPXENVptr env, CPXLPptr lp)
/*********************************************************************************************************************************************************/
{
	int lpstat = CPXgetstat (env, lp);
	if ( VERBOSE >= 100 ) printf(" CPLEX lpstat %d\n", lpstat);
	int solved = ( lpstat == CPXMIP_OPTIMAL ) ||
				 ( lpstat == CPXMIP_OPTIMAL_INFEAS ) ||
				 //( lpstat ==  CPXMIP_OPTIMAL_RELAXED ) ||
				 ( lpstat ==  CPXMIP_OPTIMAL_TOL );
	return solved;
}

/*********************************************************************************************************************************************************/
int mip_infeasible(CPXENVptr env, CPXLPptr lp)
/*********************************************************************************************************************************************************/
{
	int lpstat = CPXgetstat(env, lp);
	if ( VERBOSE >= 100 ) printf(" CPLEX lpstat %d\n", lpstat);
	int infeas = ( lpstat == CPXMIP_INFEASIBLE );
	return infeas;
}

/*********************************************************************************************************************************************************/
void mip_set_level_for_all_cuts(CPXENVptr env, int level)
/*********************************************************************************************************************************************************/
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


/*********************************************************************************************************************************************************/
void mip_change_bounds(CPXENVptr env, CPXLPptr lp, int jvar, double clb, double cub)
/*********************************************************************************************************************************************************/
{
  char low = 'L', upp = 'U';
  CPXchgbds(env, lp, 1, &jvar, &low, &clb);
  CPXchgbds(env, lp, 1, &jvar, &upp, &cub);
}

/*********************************************************************************************************************************************************/
int mip_change_rhs(CPXENVptr env, CPXLPptr lp, int irow, const double rhs)
/*********************************************************************************************************************************************************/
{
	return CPXchgrhs(env, lp, 1, &irow, &rhs);
}

/*********************************************************************************************************************************************************/
int num_cores(instance *inst)
/*********************************************************************************************************************************************************/
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

	//printf(" >>>>>>>>> mip_remove_slack_cuts() removed %d cuts\n", nrows-CPXgetnumrows(env,lp));
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


/**************************************************************************************************************************/
void double_vector_copy(int n, const double *from, double *to) // vector copy
/**************************************************************************************************************************/
{
	for ( int j = 0; j < n; j++ ) to[j] = from[j];
}

/**************************************************************************************************************************/
int *seqalloc(int n)
/**************************************************************************************************************************/
{
	int *v = (int *) malloc(n * sizeof(int));
	for ( int i = 0; i < n; i++ ) v[i] = i;
	return v;
}

/**************************************************************************************************************************/
int *int_alloc(int n, int constant)
/**************************************************************************************************************************/
{
	int *v = (int *) malloc(n * sizeof(int));
	for ( int i = 0; i < n; i++ ) v[i] = constant;
	return v;
}

/**************************************************************************************************************************/
char *char_alloc(int n, char constant)
/**************************************************************************************************************************/
{
	char *v = (char *) malloc(n * sizeof(char));
	for ( int i = 0; i < n; i++ ) v[i] = constant;
	return v;
}


/***************************************************************************************************************************************************************************************/
double mip_compute_violation(double *xstar, int nnz, int *index, double *value, char sense, double rhs)
/***************************************************************************************************************************************************************************************/
{
	double lhs = 0.0;
	for ( int k = 0; k < nnz; k++ ) lhs += value[k] * xstar[index[k]];
	if ( sense == 'E' ) return fabs(lhs-rhs);
	if ( sense == 'G' ) return dmax(0.0, rhs - lhs);
	if ( sense == 'L' ) return dmax(0.0, lhs - rhs);
	print_error("mip_compute_violation(): unknown sense??");
	return 0.0;
}

