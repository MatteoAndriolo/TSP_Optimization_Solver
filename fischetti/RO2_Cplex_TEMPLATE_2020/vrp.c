#include "vrp.h"
#include <time.h>
   
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void print_error(const char *err);
double second();       
void debug(const char *err);       
double random01() { return ((double) rand() / RAND_MAX); } // return a random value in range 0.0-1.0
int time_limit_expired(instance *inst);
int num_cores(instance *inst);   
void preprocessing(instance *inst);      
void q_heur(instance *inst);
void vrp_do_old_benders(instance *inst, CPXENVptr env, CPXLPptr lp);

double mip_value(CPXENVptr env, CPXLPptr lp);
int mip_solved_to_optimality(CPXENVptr env, CPXLPptr lp);
int mip_solution_available(CPXENVptr env, CPXLPptr lp);  
void mip_timelimit(CPXENVptr env, double timelimit, instance *inst);
int mip_update_incumbent(CPXENVptr env, CPXLPptr lp, instance *inst);
void mip_add_cutoff_constraint(CPXENVptr env, CPXLPptr lp, double cutoff);     
void mip_set_level_for_all_cuts(CPXENVptr env, int level);         
void mip_add_cut(void *env, void *cbdata, int wherefrom, int nnz, double rhs, char sense, int *index, double *value, int purgeable);

int xpos(int i, int j, instance *inst) { return inst->xstart + i * inst->nnodes + j; }                                              
int qpos(int i, int j, instance *inst) { return inst->qstart + i * inst->nnodes + j; }                                              
int bigqpos(int i, instance *inst) { return inst->bigqstart + i; }                                              
int spos(int i, int j, instance *inst) { return inst->sstart + i * inst->nnodes + j; }                                              
int bigspos(int i, instance *inst) { return inst->bigsstart + i; }                                              
int ypos(int i, int j, instance *inst) { if ( i == j ) return -1; if ( i > j ) return ypos(j,i,inst); return inst->ystart + i * (inst->nnodes+1) + j - (i+1)*(i+2)/2 ; }                                              
int fpos(int i, int j, instance *inst) { return inst->fstart + i * (inst->nnodes+1) + j; }                                              
int zpos(int i, int j, instance *inst) { if ( i == j ) return -1; if ( i > j ) return zpos(j,i,inst); return inst->zstart + i * inst->nnodes + j - (i+1)*(i+2)/2 ; }                                              

double dist(int i, int j, instance *inst)
{
	double dx = inst->xcoord[i] - inst->xcoord[j];
	double dy = inst->ycoord[i] - inst->ycoord[j]; 
	if ( !inst->integer_costs ) return sqrt(dx*dx+dy*dy);
	int dis = sqrt(dx*dx+dy*dy) + 0.499999999; // nearest integer 
	return dis+0.0;
}        

int is_fractional(double x) 						// it works for x in [0,1] only
{
	return ( (x > XSMALL) && (x < 1-XSMALL) );
}    

int is_all_integer(int n, const double *x) 			// it works for x_j in [0,1] only
{
	for ( int j = 0; j < n; j++ ) 
	{
		if ( is_fractional(x[j]) ) return 0; 
	}
	return 1;
}                                                                                                                               
                         

int time_limit_expired(instance *inst)	 
{
	double tspan = second() - inst->tstart;
	if (  tspan > inst->timelimit ) 
	{
		if ( VERBOSE >= 100 ) printf("\n\n$$$ time limit of %10.1lf sec.s expired after %10.1lf sec.s $$$\n\n", inst->timelimit, tspan);
		//exit(0); 
		return 1;
	}  
	return 0;
}

void double_vector_copy(int n, const double *from, double *to) // vector copy
{
	for ( int j = 0; j < n; j++ ) to[j] = from[j];
}


/**************************************************************************************************************************/
int VRPopt(instance *inst)
/**************************************************************************************************************************/
{  

/* 1. initialization ------------------------------------------------- */

	//time_t tt_1 = time(NULL); 		// seconds since the epoch    
	inst->tstart = second();   
	inst->num_threads = num_cores(inst);  
	inst->best_lb = -CPX_INFBOUND;   
	
	// define the bounds for load when leaving a node
	inst->load_min = (double *) calloc(inst->nnodes, sizeof(double)); 	 
	inst->load_max = (double *) calloc(inst->nnodes, sizeof(double)); 	 
	for ( int h = 0; h < inst->nnodes; h ++ ) { inst->load_min[h] = inst->demand[h]; inst->load_max[h] = inst->capacity; }
	inst->load_min[inst->depot] = inst->load_max[inst->depot] = 0.0;  
   
	// open cplex model
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);
	CPXLPptr lp = CPXcreateprob(env, &error, "VRP"); 
	
	// random seed and parallel mode
	if ( inst->randomseed != 0 ) CPXsetintparam(env, CPX_PARAM_RANDOMSEED, fabs(inst->randomseed));
	if ( 1 || ((inst->randomseed < 0) && (inst->num_threads > 1)) ) 	// **always** opportunistics mode                                                         
	{
		if ( VERBOSE >= 2 ) printf(" ... Cplex in opportunistic mode with %d thread(s)\n", inst->num_threads);
		CPXsetintparam(env, CPX_PARAM_PARALLELMODE, -1); 	
	} 

	// also move srand() seed
	if ( inst->randomseed != 0 ) { srand(7645321+abs(inst->randomseed)); for ( int k = 0; k < 1000; k++ ) random01(); } 
	
	// cplex output
	CPXsetintparam(env, CPX_PARAM_MIPDISPLAY, 4);						// 3??
	if ( VERBOSE >= 60 ) CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON); // Cplex output on screen

	// memory management
	CPXsetdblparam(env, CPX_PARAM_WORKMEM, inst->available_memory+0.0); // at XX MB write nodes on file
	if ( CPXsetstrparam(env, CPX_PARAM_WORKDIR, inst->node_file) ) print_error("wrong node file"); 	// node file name ("." or "/ext" etc.) 
	CPXsetintparam(env, CPX_PARAM_NODEFILEIND, 3);						// node file on disk and compressed
	if ( strcmp(inst->node_file,"NULL") == 0 )  
	{
		if ( VERBOSE >= 20 ) printf(" ... using cplex without node file: will stop if more than %lf MB\n", inst->available_memory+0.0);
		CPXsetdblparam(env, CPX_PARAM_TRELIM, inst->available_memory+0.0);  // max. tree size (e.g., 12 gb)
		CPXsetintparam(env, CPX_PARAM_NODEFILEIND, 0);						// no node file (for batch runs on the cluster)      
	}           
	if ( VERBOSE >= 20 ) printf(" ... using cplex node file %s after %lf MB\n", inst->node_file, inst->available_memory+0.0);
	
	// precision
	CPXsetdblparam(env, CPX_PARAM_EPINT, 0.0);		
	CPXsetdblparam(env, CPX_PARAM_EPGAP, 1e-9);	 
	CPXsetdblparam(env, CPX_PARAM_EPRHS, 1e-9);   						
	
	// initial LP algorithm
	//CPXsetintparam(env, CPX_PARAM_LPMETHOD, 4);	// barrier at the root

	// barrier vs simplex
	//CPXsetintparam(env, CPX_PARAM_STARTALG, 6); 	// concurrent, cplex bug??  
	
	// polish
	//CPXsetdblparam(env, CPX_PARAM_POLISHAFTERDETTIME, 1.0*TICKS_PER_SECOND);     
	//CPXsetintparam(env, CPX_PARAM_POLISHAFTERNODE, 0);     
	//CPXsetdblparam(env, CPX_PARAM_DETTILIM, xxx*TICKS_PER_SECOND+0.0)    
	
	// heuristics
	//CPXsetintparam(env, CPX_PARAM_FPHEUR, 1); 		// feasibility pump for feasibility (=1)  
	CPXsetintparam(env, CPX_PARAM_RINSHEUR, 10);    	// heuristic RINS frequency (=50 or alike)
	//CPXsetintparam(env, CPX_PARAM_LBHEUR, 1);			// local branching      
	//CPXsetintparam(env, CPX_PARAM_HEURFREQ, 10);    
	
	//if ( CPXsetintparam(env, CPX_PARAM_BRDIR, inst->branch) ) print_error("wrong branching flag (-1,0,1)");      
	//CPXsetintparam(env, CPX_PARAM_NODESEL, 0);			// depth first      
	
	//cuts and symmetry
	mip_set_level_for_all_cuts(env, 2); 
	//mip_set_level_for_all_cuts(env, 3); CPXsetintparam(env, CPX_PARAM_DISJCUTS, 2);
	//CPXsetintparam(env, CPX_PARAM_LOCALIMPLBD, 3);			// aggressive local implied bounds  
	//CPXsetintparam(env, CPX_PARAM_SYMMETRY, 5);			// symmetry
	
	// cutoff
	if ( inst->cutoff < CPX_INFBOUND/2.0 ) CPXsetdblparam(env, CPX_PARAM_CUTUP, inst->cutoff);
		 
/* 2. build initial model  ------------------------------------------------- */

	if ( inst->old_benders ) inst->model_type = 4;
	else q_heur(inst);   
	
	build_model(inst, env, lp);
	//postprocessing(inst, env, lp);    
	
	int ncols = CPXgetnumcols(env, lp);
	inst->best_sol = (double *) calloc(ncols, sizeof(double)); 	// all entries to zero  
	inst->zbest = CPX_INFBOUND;  

/* 3. final MIP run ------------------------------------------------------ */

	if ( inst->old_benders ) 
	{
		vrp_do_old_benders(inst, env, lp); 
	}
	else 
	{
		mip_timelimit(env, CPX_INFBOUND, inst);
		if ( VERBOSE >= 50 ) CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON); 
		if ( time_limit_expired(inst) ) goto EXIT;           
		if ( VERBOSE >= 100 ) CPXwriteprob(env, lp, "final.lp", NULL);
		//CPXsetintparam(env, CPX_PARAM_NODELIM, 0); // 0 and not 1      
		//for ( int k = 0; k < 10; k++ ) CPXmipopt(env,lp); // 10 restarts 
		//CPXsetintparam(env, CPX_PARAM_NODELIM, 2000000000);    
		if ( inst->max_nodes >= 0 ) CPXsetintparam(env, CPX_PARAM_NODELIM, inst->max_nodes);      
		CPXmipopt(env,lp);     
	}

/* 99. final statistics ------------------------------------------------- */
	
EXIT:
	
	CPXgetbestobjval(env, lp, &inst->best_lb); 
	mip_update_incumbent(env, lp, inst);                 

	//double tot_time = second()-inst->tstart;    
	//time_t tt_2 = time(NULL); 		// seconds since the epoch

	//printf("\n\n >> %s --> best value %lf init_lb %lf best lower bound %lf after %10.2lf sec.s [opt %1d timlim %1d nodes %7d %%igap %6.2lf %%fgap %6.2lf] seed %d\n", inst->input_file, inst->zbest, init_lb, inst->best_lb, second()-inst->tstart, opt, time_limit_expired(inst), nodecount, 100*igap, 100*gap, inst->randomseed);
	//printf("\n STAH; input_file ; n_locations ; n_clients ; seed ; zbest ; final_lb ; init_lb ; root_bound; time (s.); root_time (s.); opt ; timelimit ; nodes ; %%ini_gap ; %%root_gap; %%final_gap ; final_cut ; USER_CUTS ; QUADRATIC ; init_time ; init_ncuts ; heu_time ; heu_val ; branch ; cut_purge ; my_worker ; num_workers ; sample_nodes ; -benders ; time_start ; time_end ; delta_time ; true_cost; %%sep_time ; ysum ; varfixing ; RECOMPUTE_DUAL_IN_BENDERS ; cpxnet ; varsel ; old_benders ;\n");
	//printf(" STAT; %s ; %d ; %d ; %d ; %lf ; %lf ; %lf ; %lf ; %lf ; %lf ; %d ; %d ; %d ; %lf ; %lf ; %lf ; %d ; %d ; %d ; %lf ; %d ; %lf ; %lf ; %d ; %d ; %d ; %d ; %d ;  %d ; %ld ; %ld ; %ld ; %lf ; %lf ; %d ; %d ; %d ; %d ; %d ; %d ; \n\n", inst->input_file, inst->n_locations, inst->n_clients, inst->randomseed, inst->zbest, inst->best_lb, init_lb, inst->root_bound.z, tot_time, inst->root_bound.t, opt , time_limit_expired(inst), nodecount, 100*igap, 100*rootgap, 100*gap, inst->final_cut, inst->user_cuts, is_quadratic(inst), init_time , init_ncuts, heu_time, heu_val, inst->branch, inst->cut_purge, inst->my_worker, inst->num_workers, inst->sample_nodes, inst->benders_capacitated, tt_1, tt_2, tt_2-tt_1, true_value, 100.0*tm/tot_time, inst->ysum, inst->varfixing, RECOMPUTE_DUAL_IN_BENDERS, cpxnet, inst->varsel, inst->old_benders);

	// free pools and close cplex model   
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env); 	
	
	return 0;
}  
 

/**************************************************************************************************************************/
void preprocessing(instance *inst)
/**************************************************************************************************************************/
{                         

	double t1 = second();
	double zero = 0.0, one = 1.0;
	// open cplex model
	int error;
	CPXENVptr env = CPXopenCPLEX(&error); 
	CPXLPptr lp = CPXcreateprob(env, &error, "preVRP"); 
	build_model(inst, env, lp);        
	if ( inst->cutoff < CPX_INFBOUND/2.0 ) mip_add_cutoff_constraint(env, lp, inst->cutoff);  
	int ncols = CPXgetnumcols(env, lp);
	for ( int j = 0; j < ncols; j++ ) CPXchgobj(env, lp, 1, &j, &zero); 
	
	if ( VERBOSE >= 100 ) CPXwriteprob(env, lp, "pre.lp", NULL); 

	CPXsetintparam(env, CPX_PARAM_NODELIM, 100000000); 
	if ( VERBOSE >= 20 ) CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON); // Cplex output on screen
	
	int last_changed = -1; 
	int counter = 0;
	
	int h = -1;
	while ( inst->bigqstart >= 0 ) 
	{
		if ( ++h >= inst->nnodes ) h = 0;        
		counter++;
		if ( inst->load_min[h] >= inst->load_max[h] - EPSILON ) continue;
		if ( h == last_changed || counter > inst->nnodes ) break;

		int pos = bigqpos(h, inst);
		CPXchgobj(env, lp, 1, &pos, &one);
		CPXmipopt(env,lp);     
		double lb; CPXgetbestobjval(env, lp, &lb);
		CPXchgobj(env, lp, 1, &pos, &zero); 

		if ( VERBOSE >= 20 ) printf("preprocessing: minimum load for node %4d from %10.1lf to %10.1lf after %7.1lf sec.s ####", h+1, inst->load_min[h], lb, second()-t1);
		if ( lb > inst->load_min[h]+0.01 )
		{
			if ( VERBOSE >= 20 ) printf(" * \n"); 
			last_changed = h;
			inst->load_min[h] = lb;   
			inst->load_max[h] = dmin( inst->load_max[h], inst->capacity-(inst->load_min[h]-inst->demand[h]) ); // assuming symmetric costs
			CPXfreeprob(env, &lp);     
			lp = CPXcreateprob(env, &error, "preVRP"); 
			build_model(inst, env, lp);     
			if ( inst->cutoff < CPX_INFBOUND/2.0 ) mip_add_cutoff_constraint(env, lp, inst->cutoff);
			for ( int j = 0; j < ncols; j++ ) CPXchgobj(env, lp, 1, &j, &zero);   
			counter = 0;
		}
		else   
		{
			if ( VERBOSE >= 20 ) printf("\n");
	    }
	}

	// free and close cplex model   
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env); 	
	
}     

/**************************************************************************************************************************/
void build_model0(instance *inst, CPXENVptr env, CPXLPptr lp)   // basic model with asymmetric x and q var.s
/**************************************************************************************************************************/
{
	  
	double zero = 0.0; // one = 1.0; 	
	char binary = 'B'; 
	char continuous = 'C';
	//char integer = 'I';

	char **cname = (char **) calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = (char *) calloc(100, sizeof(char));

	// add binary var.s x(i,j)   
	if ( inst->xstart != -1 ) print_error(" ... error in build_model(): var. x cannot be redefined!");
	inst->xstart = CPXgetnumcols(env,lp); 		// position of the first x(,) variable   
	for ( int i = 0; i < inst->nnodes; i++ )
	{
		for ( int j = 0; j < inst->nnodes; j++ )
		{
			sprintf(cname[0], "x(%d,%d)", i+1,j+1);
			double obj = dist(i,j,inst);   
			double ub = ( i == j ) ? 0.0 : 1.0;
			if ( CPXnewcols(env, lp, 1, &obj, &zero, &ub, &binary, cname) ) print_error(" wrong CPXnewcols on x var.s");
    		if ( CPXgetnumcols(env,lp)-1 != xpos(i,j, inst) ) print_error(" wrong position for x var.s");
		}
	} 

	if ( inst->load_max[inst->depot] > 1e-20 ) print_error(" wrong inst->load_max[inst->depot]");
	
	// add continuous var.s bigq(i) = load when leaving node i  
	if ( inst->bigqstart != -1 ) print_error(" ... error in build_model(): var. bigq cannot be redefined!");
	inst->bigqstart = CPXgetnumcols(env,lp); 		// position of the first bigq() variable   
	for ( int i = 0; i < inst->nnodes; i++ )
	{
		sprintf(cname[0], "bigq(%d)", i+1);
		double obj = 0.0;
		double lb = inst->load_min[i];
		double ub = inst->load_max[i];
		if ( CPXnewcols(env, lp, 1, &obj, &lb, &ub, &continuous, cname) ) print_error(" wrong CPXnewcols on bigq var.s");
		//if ( CPXnewcols(env, lp, 1, &obj, &lb, &ub, &integer, cname) ) print_error(" wrong CPXnewcols on bigq var.s");
    	if ( CPXgetnumcols(env,lp)-1 != bigqpos(i, inst) ) print_error(" wrong position for bigq var.s");
	} 
	
	// add continuous var.s q(i,j) = bigq(i) * x(i,j)  
	if ( inst->qstart != -1 ) print_error(" ... error in build_model(): var. q cannot be redefined!");
	inst->qstart = CPXgetnumcols(env,lp); 		// position of the first q(,) variable   
	for ( int i = 0; i < inst->nnodes; i++ )
	{
		for ( int j = 0; j < inst->nnodes; j++ )
		{
			sprintf(cname[0], "q(%d,%d)", i+1,j+1);
			double obj = 0.0;
			double ub = ( i == j ) ?  0.0 : inst->load_max[i];
			if ( CPXnewcols(env, lp, 1, &obj, &zero, &ub, &continuous, cname) ) print_error(" wrong CPXnewcols on q var.s");
			//if ( CPXnewcols(env, lp, 1, &obj, &zero, &ub, &integer, cname) ) print_error(" wrong CPXnewcols on q var.s");
    		if ( CPXgetnumcols(env,lp)-1 != qpos(i,j, inst) ) print_error(" wrong position for q var.s");
		}
	} 

	// x-constraints (out- and in-degree at nodes)
	for ( int h = 0; h < inst->nnodes; h++ )  // out-degree
	{
		int lastrow = CPXgetnumrows(env,lp);
		double rhs = ( h == inst->depot ) ? inst->nveh : 1.0; 
		if ( rhs < 0.1 ) continue;		// means that the n. of vehicles is not fixed
		char sense = 'E'; 
		sprintf(cname[0], "outdeg(%d)", h+1);   
		if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [x1]");
		for ( int i = 0; i < inst->nnodes; i++ )
		{
			if ( CPXchgcoef(env, lp, lastrow, xpos(i,h, inst), 1.0) ) print_error(" wrong CPXchgcoef [x1]");
		}
	}  
	for ( int h = 0; h < inst->nnodes; h++ ) // in-degree
	{
		int lastrow = CPXgetnumrows(env,lp);
		double rhs = ( h == inst->depot ) ? inst->nveh : 1.0;
		if ( rhs < 0.1 ) continue;
		char sense = 'E'; 
		sprintf(cname[0], "indeg(%d)", h+1);
		if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [x2]");
		for ( int i = 0; i < inst->nnodes; i++ )
		{
			if ( CPXchgcoef(env, lp, lastrow, xpos(h,i, inst), 1.0) ) print_error(" wrong CPXchgcoef [x2]");
		}
	}  
	
	// q-constraints  
	for ( int h = 0; h < inst->nnodes; h++ ) // define bigq(h) = \sum_j q(h,j)
	{
		int lastrow = CPXgetnumrows(env,lp);
		double rhs = 0.0;
		char sense = 'E';      
		sprintf(cname[0], "def_bigq(%d)", h+1);
		if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [q1]");
		if ( CPXchgcoef(env, lp, lastrow, bigqpos(h, inst), -1.0) ) print_error(" wrong CPXchgcoef [q1]");
		for ( int j = 0; j < inst->nnodes; j++ )  if ( CPXchgcoef(env, lp, lastrow, qpos(h,j, inst), 1.0) ) print_error(" wrong CPXchgcoef [q2]");
	}    
	for ( int h = 0; h < inst->nnodes; h++ ) // flow constraint bigq(h) = demand(h) + \sum_i q(i,h)
	{
		if ( h == inst->depot ) continue;	// skip depot
		int lastrow = CPXgetnumrows(env,lp);
		double rhs = inst->demand[h];
		char sense = 'E'; 
		sprintf(cname[0], "flow(%d)", h+1);
		if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [bq1]");
		if ( CPXchgcoef(env, lp, lastrow, bigqpos(h, inst), 1.0) ) print_error(" wrong CPXchgcoef [bq1]");
		for ( int i = 0; i < inst->nnodes; i++ ) if ( CPXchgcoef(env, lp, lastrow, qpos(i,h, inst), -1.0) ) print_error(" wrong CPXchgcoef [bq2]");
	}    
 
	// q-x linking 
	for ( int i = 0; i < inst->nnodes; i++ )
	{
		if ( i == inst->depot ) continue;
		for ( int j = 0; j < inst->nnodes; j++ )     // q(i,j) [= bigq(i) * x(i,j)] <= [capacity-demand(j)] x(i,j)
		{
			if ( i == j ) continue;
			int lastrow = CPXgetnumrows(env,lp);
			double rhs = 0.0;
			char sense = 'L'; 
			sprintf(cname[0], "link(%d,%d)", i+1,j+1);    
			double coef = inst->load_max[i];
			if ( j != inst->depot ) coef = dmin(coef, inst->capacity-inst->demand[j]);
			if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [l1]");
			if ( CPXchgcoef(env, lp, lastrow, qpos(i,j, inst), 1.0) ) print_error(" wrong CPXchgcoef [l2]");
			if ( CPXchgcoef(env, lp, lastrow, xpos(i,j, inst), -coef) ) print_error(" wrong CPXchgcoef [l3]");
		}
	}  
	for ( int i = 0; i < inst->nnodes; i++ )
	{
		if ( i == inst->depot ) continue;
		for ( int j = 0; j < inst->nnodes; j++ )   // q(i,j) [= bigq(i) * x(i,j)] >= load_min(i) x(i,j)
		{
			if ( i == j ) continue;
			int lastrow = CPXgetnumrows(env,lp);
			double rhs = 0.0;
			char sense = 'G'; 
			sprintf(cname[0], "link2(%d,%d)", i+1,j+1);
			if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [l1]");
			if ( CPXchgcoef(env, lp, lastrow, qpos(i,j, inst), 1.0) ) print_error(" wrong CPXchgcoef [l2]");
			if ( CPXchgcoef(env, lp, lastrow, xpos(i,j, inst), -inst->load_min[i]) ) print_error(" wrong CPXchgcoef [l3]");
		}
	} 
	
	free(cname[0]);
	free(cname);
	   
}

/**************************************************************************************************************************/
void build_model1(instance *inst, CPXENVptr env, CPXLPptr lp)    // add SOC contraints to model 0
/**************************************************************************************************************************/
{
	double zero = 0.0; // one = 1.0; 	
	char continuous = 'C';
	//char binary = 'B'; 
	//char integer = 'I';

	char **cname = (char **) calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = (char *) calloc(100, sizeof(char));

	double sobj = 0.0;  ///////////////////////// 0.01 ??

	if ( fabs(sobj) > EPSILON && inst->cutoff < CPX_INFBOUND/2.0 ) 
	{
		mip_add_cutoff_constraint(env, lp, inst->cutoff);  
		CPXsetdblparam(env, CPX_PARAM_CUTUP, CPX_INFBOUND);
	}	
	
	// add continuous var.s s(i,j) = q(i,j)^2 = bigq(i)^2 x(i,j)^2 
	if ( inst->sstart != -1 ) print_error(" ... error in build_model(): var. s cannot be redefined!");
	inst->sstart = CPXgetnumcols(env,lp); 		// position of the first s(,) variable   
	for ( int i = 0; i < inst->nnodes; i++ )
	{
		for ( int j = 0; j < inst->nnodes; j++ )
		{
			sprintf(cname[0], "s(%d,%d)", i+1,j+1);
			double ub = ( i == inst->depot ) ? 0.0 : inst->load_max[i]* inst->load_max[i];
			if ( CPXnewcols(env, lp, 1, &sobj, &zero, &ub, &continuous, cname) ) print_error(" wrong CPXnewcols on s var.s");
    		if ( CPXgetnumcols(env,lp)-1 != spos(i,j, inst) ) print_error(" wrong position for s var.s");
		}
	} 

#if 1
	// add continuous var.s bigs(i) = bigq(i)^2  
	if ( inst->bigsstart != -1 ) print_error(" ... error in build_model(): var. bigs cannot be redefined!");
	inst->bigsstart = CPXgetnumcols(env,lp); 		// position of the first bigq() variable   
	for ( int i = 0; i < inst->nnodes; i++ )
	{
		sprintf(cname[0], "bigs(%d)", i+1);
		double obj = 0.0;
		double lb = ( i == inst->depot ) ? 0.0 : inst->load_min[i]*inst->load_min[i];
		double ub = ( i == inst->depot ) ? 0.0 : inst->load_max[i]*inst->load_max[i]; 
		if ( CPXnewcols(env, lp, 1, &obj, &lb, &ub, &continuous, cname) ) print_error(" wrong CPXnewcols on bigs var.s");
    	if ( CPXgetnumcols(env,lp)-1 != bigspos(i, inst) ) print_error(" wrong position for bigs var.s");
	}   
	
	// s-constraints  
	for ( int h = 0; h < inst->nnodes; h++ )    // bigs(h) = \sum_j s(h,j)
	{
		int lastrow = CPXgetnumrows(env,lp);
		double rhs = 0.0;
		char sense = 'E';      
		sprintf(cname[0], "def_bigs(%d)", h+1);
		if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [s1]");
		if ( CPXchgcoef(env, lp, lastrow, bigspos(h, inst), -1.0) ) print_error(" wrong CPXchgcoef [s1]");
		for ( int j = 0; j < inst->nnodes; j++ ) if ( CPXchgcoef(env, lp, lastrow, spos(h,j, inst), 1.0) ) print_error(" wrong CPXchgcoef [s2]");
	}    
	for ( int h = 0; h < inst->nnodes; h++ )  //  bigs(h) = (demand[h] + \sum_i q(i,h))^2 = demand[h]^2 + sum_i s(i,h) + 2 * demand[h] * sum_i q(i,h)) 
	{
		if ( h == inst->depot ) continue;
		int lastrow = CPXgetnumrows(env,lp);
		double rhs = inst->demand[h]*inst->demand[h];
		char sense = 'E'; 
		sprintf(cname[0], "sflow(%d)", h+1);
		if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [bq1]");
		if ( CPXchgcoef(env, lp, lastrow, bigspos(h, inst), 1.0) ) print_error(" wrong CPXchgcoef [bq1]");
		for ( int i = 0; i < inst->nnodes; i++ )
		{
			if ( CPXchgcoef(env, lp, lastrow, spos(i,h, inst), -1.0) ) print_error(" wrong CPXchgcoef [bq2]");
			if ( CPXchgcoef(env, lp, lastrow, qpos(i,h, inst), -2.0*inst->demand[h]) ) print_error(" wrong CPXchgcoef [bq2]");
		}
	}  
#endif		

	// soc's  q(i,j)^2 - s(i,j) x(i,j) <= 0 for all i,j    
	int linnzcnt = 0;  
	int quadnzcnt = 2;
	double rhsval = 0.0;
	char sense = 'L';                                                   
	int linind[1] = { -1 };
	double linval[1] = { 0.0 };
	int quadrow[2];
	int quadcol[2];
	double quadval[2] = { 1.0, -1.0 };
	for ( int i = 0; i < inst->nnodes; i++ )
	{
		if ( i == inst->depot ) continue;
		for ( int j = 0; j < inst->nnodes; j++ )
		{
			if ( i == j ) continue;
			sprintf(cname[0], "soc(%d,%d)", i+1,j+1);
			quadrow[0] = quadcol[0] = qpos(i,j, inst); quadrow[1] = spos(i,j, inst); quadcol[1] = xpos(i,j, inst);
			if ( CPXaddqconstr(env, lp, linnzcnt, quadnzcnt, rhsval, sense, linind, linval, quadrow, quadcol, quadval, cname[0]) ) print_error(" wrong CPXaddqconstr [1]");
		}
	}
	
	free(cname[0]);
	free(cname);
} 


/***************************************************************************************************************************/
void build_model2(instance *inst, CPXENVptr env, CPXLPptr lp) // R. Baldacci, E. Hadjiconstantinou and A. Mingozzi. (2004) model
/**************************************************************************************************************************/
{

	//if ( inst->nveh  <= 0 ) print_error(" model 2 cannot be applied when n. vehicles is unspecified");
	    
	double zero = 0.0; // one = 1.0; 	
	char binary = 'B'; 
	char continuous = 'C';
	char integer = 'I';

	char **cname = (char **) calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = (char *) calloc(100, sizeof(char));

	int numext = inst->nnodes + 1; 	// n. of nodes in the extended digraph   
	double qsum = 0.0; 
	for ( int i = 0; i < inst->nnodes; i++ ) if ( i != inst->depot ) qsum += inst->demand[i];     
	
	// add integer var. nveh 
	int nvehpos = CPXgetnumcols(env,lp);
	sprintf(cname[0], "nveh"); 
	double llb = ( inst->nveh > 0 ) ? inst->nveh : int(qsum / inst->capacity + 0.999999); 
	double uub = ( inst->nveh > 0 ) ? inst->nveh : inst->nnodes; 
	if ( CPXnewcols(env, lp, 1, &zero, &llb, &uub, &integer, cname) ) print_error(" wrong CPXnewcols on nveh var");
	if ( CPXgetnumcols(env,lp)-1 != nvehpos ) print_error(" wrong position for nveh var.s");
	
	
	// add binary var.s y(i,j)   
	if ( inst->ystart != -1 ) print_error(" ... error in build_model(): var. y cannot be redefined!");
	inst->ystart = CPXgetnumcols(env,lp); 		// position of the first y(,) variable   
	for ( int i = 0; i < numext; i++ )
	{
		int ii = ( i == numext-1 ) ? inst->depot : i;
		for ( int j = i+1; j < numext; j++ )
		{
			sprintf(cname[0], "y(%d,%d)", i+1,j+1);  
			int jj = ( j == numext-1 ) ? inst->depot : j;
			double obj = dist(ii,jj,inst);   
			double ub = 1.0;   
			char type = binary; //if ( i == inst->depot || j == numext-1 ) type = continuous; // to avoid symmetry on the binary var.s
			if ( CPXnewcols(env, lp, 1, &obj, &zero, &ub, &type, cname) ) print_error(" wrong CPXnewcols on y var.s");
    		if ( CPXgetnumcols(env,lp)-1 != ypos(i,j, inst) ) print_error(" wrong position for y var.s");
		}
	} 

	// add continuous var.s f(i,j)  
	if ( inst->fstart != -1 ) print_error(" ... error in build_model(): var. f cannot be redefined!");
	inst->fstart = CPXgetnumcols(env,lp); 		// position of the first q(,) variable   
	for ( int i = 0; i < numext; i++ )
	{
		for ( int j = 0; j < numext; j++ )
		{
			sprintf(cname[0], "f(%d,%d)", i+1,j+1);
			double obj = 0.0;
			double ub = ( i == j ) ? 0.0 : CPX_INFBOUND; //inst->capacity;    
			if ( i == inst->depot && j == numext-1 ) ub = 0;
			if ( j == inst->depot && i == numext-1 ) ub = 0;
			if ( CPXnewcols(env, lp, 1, &obj, &zero, &ub, &continuous, cname) ) print_error(" wrong CPXnewcols on f var.s");
			//if ( CPXnewcols(env, lp, 1, &obj, &zero, &ub, &integer, cname) ) print_error(" wrong CPXnewcols on q var.s");
			if ( CPXgetnumcols(env,lp)-1 != fpos(i,j, inst) ) print_error(" wrong position for f var.s");
		}
	} 

	// eq. (28) in the OpRes paper     
	for ( int i = 0; i < inst->nnodes; i++ ) 
	{
		if ( i == inst->depot ) continue;	// skip depot
		int lastrow = CPXgetnumrows(env,lp);
		double rhs = 2.0*inst->demand[i];
		char sense = 'E'; 
		sprintf(cname[0], "flow(%d)", i+1);
		if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [f1]");
		for ( int j = 0; j < numext; j++ ) 
		{
			if ( CPXchgcoef(env, lp, lastrow, fpos(j,i, inst), +1.0) ) print_error(" wrong CPXchgcoef [f3]");
			if ( CPXchgcoef(env, lp, lastrow, fpos(i,j, inst), -1.0) ) print_error(" wrong CPXchgcoef [f2]");
		}
	} 

	// eq. (29)  
	{    
		int lastrow = CPXgetnumrows(env,lp);	
		double rhs = qsum;
		char sense = 'E'; 
		sprintf(cname[0], "outflow_dep(%d)", inst->depot+1);
		if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [f1a]");
		for ( int j = 0; j < inst->nnodes; j++ ) if ( j != inst->depot  && CPXchgcoef(env, lp, lastrow, fpos(inst->depot,j, inst), 1.0) ) print_error(" wrong CPXchgcoef [f2a]");
	}

	// eq. (30)      
	{
		int lastrow = CPXgetnumrows(env,lp);	
		double rhs = inst->nveh * inst->capacity - qsum;  
		if ( inst->nveh <= 0 ) rhs = -qsum;
		char sense = 'E'; 
		sprintf(cname[0], "inflow_dep(%d)", inst->depot+1);
		if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [f1b]");
		for ( int j = 0; j < inst->nnodes; j++ ) if ( j != inst->depot  && CPXchgcoef(env, lp, lastrow, fpos(j,inst->depot, inst), 1.0) ) print_error(" wrong CPXchgcoef [f1b]");
		if ( inst->nveh <= 0 && CPXchgcoef(env, lp, lastrow, nvehpos, -inst->capacity) ) print_error(" wrong CPXchgcoef [f1c]");
	}
	
	// eq. (31)      
	{
		int lastrow = CPXgetnumrows(env,lp);	
		double rhs = inst->nveh * inst->capacity;
		if ( inst->nveh <= 0 ) rhs = 0.0;
		char sense = 'E'; 
		sprintf(cname[0], "flow_extranode(%d)", inst->nnodes+1);
		if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [f2a]");
		for ( int j = 0; j < inst->nnodes; j++ ) if ( j != inst->depot  && CPXchgcoef(env, lp, lastrow, fpos(numext-1,j, inst), 1.0) ) print_error(" wrong CPXchgcoef [f2b]");
		if ( inst->nveh <= 0 && CPXchgcoef(env, lp, lastrow, nvehpos, -inst->capacity) ) print_error(" wrong CPXchgcoef [f2c]");
    }

	// eq. (32)      
	for ( int i = 0; i < numext; i++ )
	{
		for ( int j = i+1; j < numext; j++ )
		{
			int lastrow = CPXgetnumrows(env,lp);	
			double rhs = 0.0;
			char sense = 'E'; 
			sprintf(cname[0], "linking(%d,%d)", i+1,j+1);
			if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [l1]"); 
			if ( CPXchgcoef(env, lp, lastrow, ypos(i,j, inst), -inst->capacity) ) print_error(" wrong CPXchgcoef [l2]");
			if ( CPXchgcoef(env, lp, lastrow, fpos(i,j, inst), 1.0) ) print_error(" wrong CPXchgcoef [l3]");
			if ( CPXchgcoef(env, lp, lastrow, fpos(j,i, inst), 1.0) ) print_error(" wrong CPXchgcoef [l4]");
		}
	}
	
	// eq. (33) 
	for ( int i = 0; i < inst->nnodes; i++ )  // degree
	{
		if ( i == inst->depot ) continue;
		int lastrow = CPXgetnumrows(env,lp);
		double rhs = 2.0; 
		char sense = 'E'; 
		sprintf(cname[0], "degree(%d)", i+1);   
		if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [d1]");
		for ( int j = 0; j < numext; j++ ) if ( i != j && CPXchgcoef(env, lp, lastrow, ypos(i,j, inst), 1.0) ) print_error(" wrong CPXchgcoef [d2]");
	}  
	
	// define nveh var.
	if ( inst->nveh <= 0 )
	{
		int lastrow = CPXgetnumrows(env,lp);
		double rhs = 0.0; 
		char sense = 'E'; 
		sprintf(cname[0], "def_nveh");   
		if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [v1]");  
		if ( CPXchgcoef(env, lp, lastrow, nvehpos, -1.0) ) print_error(" wrong CPXchgcoef [v2]");
		for ( int j = 0; j < inst->nnodes; j++ ) if ( j != inst->depot && CPXchgcoef(env, lp, lastrow, ypos(inst->depot,j, inst), 1.0) ) print_error(" wrong CPXchgcoef [v3]");
	}    
	
	free(cname[0]);
	free(cname);
	
}   


/***************************************************************************************************************************/
void build_model3(instance *inst, CPXENVptr env, CPXLPptr lp) // double model: 0 plus 2
/**************************************************************************************************************************/
{

	double zero = 0.0; // one = 1.0; 	

	char **cname = (char **) calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = (char *) calloc(100, sizeof(char));  
	
	build_model0(inst, env,lp);
	int ncols = CPXgetnumcols(env, lp);
	for ( int j = 0; j < ncols; j++ ) CPXchgobj(env, lp, 1, &j, &zero); 
	build_model2(inst, env,lp);
	  
	for ( int i = 0; i < inst->nnodes; i++ )
	{
		if ( i == inst->depot ) continue;
		for ( int j = i+1; j < inst->nnodes; j++ )
		{
			if ( j == inst->depot ) continue;
			int lastrow = CPXgetnumrows(env,lp);	
			double rhs = 0.0;
			char sense = 'E'; 
			sprintf(cname[0], "equal(%d,%d)", i+1,j+1);
			if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [e1]"); 
			if ( CPXchgcoef(env, lp, lastrow, ypos(i,j, inst), -1.0) ) print_error(" wrong CPXchgcoef [e2]");
			if ( CPXchgcoef(env, lp, lastrow, xpos(i,j, inst), 1.0) ) print_error(" wrong CPXchgcoef [e3]");
			if ( CPXchgcoef(env, lp, lastrow, xpos(j,i, inst), 1.0) ) print_error(" wrong CPXchgcoef [e4]");
		}
	}

	free(cname[0]);
	free(cname);
}    

/***************************************************************************************************************************/
void build_model4(instance *inst, CPXENVptr env, CPXLPptr lp) // very basic model with undirected y[i,j] var.s
/**************************************************************************************************************************/
{

	if ( inst->nveh  <= 0 ) print_error(" model 4 cannot be applied when n. vehicles is unspecified");
	    
	double zero = 0.0; // one = 1.0; 	
	char binary = 'B'; 

	char **cname = (char **) calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = (char *) calloc(100, sizeof(char));

	// add binary var.s y(i,j)   
	if ( inst->zstart != -1 ) print_error(" ... error in build_model(): var. y cannot be redefined!");
	inst->zstart = CPXgetnumcols(env,lp); 		// position of the first y(,) variable   
	for ( int i = 0; i < inst->nnodes; i++ )
	{
		for ( int j = i+1; j < inst->nnodes; j++ )
		{
			sprintf(cname[0], "z(%d,%d)", i+1,j+1);  
			double obj = dist(i,j,inst);   
			double ub = 1.0;   
			if ( CPXnewcols(env, lp, 1, &obj, &zero, &ub, &binary, cname) ) print_error(" wrong CPXnewcols on z var.s");
    		if ( CPXgetnumcols(env,lp)-1 != zpos(i,j, inst) ) print_error(" wrong position for z var.s");
		}
	} 

	for ( int i = 0; i < inst->nnodes; i++ )  // degree
	{
		int lastrow = CPXgetnumrows(env,lp);
		double rhs = ( i == inst->depot ) ? 2.0 * inst->nveh : 2.0; 
		char sense = 'E'; 
		sprintf(cname[0], "degree(%d)", i+1);   
		if ( CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname) ) print_error(" wrong CPXnewrows [z1]");
		for ( int j = 0; j < inst->nnodes; j++ ) if ( i != j && CPXchgcoef(env, lp, lastrow, zpos(i,j, inst), 1.0) ) print_error(" wrong CPXchgcoef [z1]");
	}  
	
	
	free(cname[0]);
	free(cname);
	
}   


/***************************************************************************************************************************/
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp)
/**************************************************************************************************************************/
{    
	inst->xstart = -1;
	inst->qstart = -1;
	inst->bigqstart = -1;  
	inst->sstart = -1;
	inst->bigsstart = -1;  
	inst->ystart = -1;
	inst->fstart = -1;
	inst->zstart = -1;

	switch (inst->model_type) 	
	{
		case 0 : 								// basic model with asymmetric x and q
				build_model0(inst, env,lp);
				break;
				  
		case 1 : 								// soc constraints (just to try...)
				build_model0(inst, env,lp);
				build_model1(inst, env,lp);
				break; 

		case 2 : 								// OpRes model
			  	build_model2(inst, env,lp);
				break; 

		case 3 : 								// mixed
				build_model3(inst, env,lp);
				break; 
				
		case 4 : 								// basic undirected model with y-var.s only (for old Benders)
				build_model4(inst, env,lp);
				break;      
				
		default: 
				print_error(" model type unknown!!"); 
				break;
		
	}  

	if ( inst->cutoff < CPX_INFBOUND/2.0 ) CPXsetdblparam(env, CPX_PARAM_CUTUP, inst->cutoff); 
	
	if ( VERBOSE >= -100 ) CPXwriteprob(env, lp, "model.lp", NULL); 
	
}  

void q_heur(instance *inst)
{
	if ( inst->old_benders) return;
	double const delta = 0.5; 
	
	if ( inst->model_type >= 2 ) print_error("q_heur() requires bigq var.s");   
	
	int error;
	CPXENVptr env = CPXopenCPLEX(&error); 
	CPXLPptr lp = CPXcreateprob(env, &error, "preVRP"); 
	build_model(inst, env, lp);        
	   
	CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
	/////////CPXsetintparam(env, CPX_PARAM_INTSOLLIM, 1); 
	CPXsetintparam(env, CPX_PARAM_RINSHEUR, 1);    	// heuristic RINS frequency (=50 or alike)
	mip_set_level_for_all_cuts(env, 2);             
	CPXsetintparam(env, CPX_PARAM_NODELIM, 1); 
	
	
	CPXmipopt(env,lp);
	
	double *bigqstar = (double *) calloc(inst->nnodes, sizeof(double));
	int fpos = bigqpos(0, inst);
	CPXgetx(env, lp, bigqstar, fpos, fpos+inst->nnodes-1);
	if ( VERBOSE >= -100 ) for ( int j = 0; j < inst->nnodes; j++ ) printf(" ... qstar[%3d] = %10.2lf (%10.2lf -- %10.2lf)\n", j+1, bigqstar[j], inst->load_min[j], inst->load_max[j]);

	// free and close cplex model   
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env); 	
	 	 
	for ( int j = 0; j < inst->nnodes; j++ ) 
	{ 
		if ( j == inst->depot ) continue;
#if 1
		inst->load_min[j] = dmax(inst->load_min[j], (1.0-delta)*bigqstar[j]);
		inst->load_max[j] = dmin(inst->load_max[j], (1.0+delta)*bigqstar[j]);
#else  
		inst->load_min[j] += int(delta * ( bigqstar[j] - inst->load_min[j] ));
		inst->load_max[j] -= int(delta * ( inst->load_max[j] - bigqstar[j] ));
#endif
	}
	if ( VERBOSE >= -100 ) for ( int j = 0; j < inst->nnodes; j++ ) printf(" new ... qstar[%3d] = %10.2lf (%10.2lf -- %10.2lf)\n", j+1, bigqstar[j], inst->load_min[j], inst->load_max[j]);
	free(bigqstar);

	env = CPXopenCPLEX(&error); 
	lp = CPXcreateprob(env, &error, "preVRP"); 
	build_model(inst, env, lp);      
	mip_set_level_for_all_cuts(env, 2); 
	 
	CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
	CPXmipopt(env,lp);

	// free and close cplex model   
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env); 	
	
	exit(1);

} 


/***************************************************************************************************************************/
int add_benders_cuts(instance *inst, CPXENVptr env, CPXLPptr lp)
/***************************************************************************************************************************/
{                
	int ncuts = 0;   
	int ncols = CPXgetnumcols(env, lp);
	double *xstar = (double *) calloc(ncols, sizeof(double));
	if ( CPXgetx(env, lp, xstar, 0, ncols-1) ) print_error("add_benders_cuts: no solution available?");   
	
	// define the connected components (without the depot)
	int *comp = (int *) malloc(inst->nnodes * sizeof(int));
	for ( int i = 0; i < inst->nnodes; i++ ) comp[i] = i;		// all singletons
	for ( int i = 0; i < inst->nnodes; i++ )
	{                                 
		if  ( i == inst->depot ) continue;
		for ( int j = i+1; j < inst->nnodes; j++ )
		{
			if  ( j == inst->depot ) continue;
			if ( xstar[zpos(i,j, inst)] < 0.5 ) continue;		// xstar is 0-1      
			if ( comp[i] == comp[j] ) continue;
			int comp_i = comp[i], comp_j = comp[j]; 
			for ( int h = 0; h < inst->nnodes; h++ ) if ( comp[h] == comp_j ) comp[h] = comp_i; // slow O(n^2) set union
		}
	}          
	
	// compute the load for each cluster
	int *nedges = (int *) calloc(inst->nnodes, sizeof(int));
	double *sum_demand = (double *) calloc(inst->nnodes, sizeof(double));	  
	for ( int i = 0; i < inst->nnodes; i++ ) 
	{
		if ( i == inst->depot ) continue;
		if ( xstar[zpos(inst->depot,i,inst)] > 0.5 ) ++nedges[comp[i]]; 
		sum_demand[comp[i]] += inst->demand[i];  
	}
	
   // generate the cuts (at most one for each component)
	int nnz;
	double rhs;     
	char sense = 'L';
	double *value = (double *) malloc(ncols * sizeof(double));
	int *index = (int *) malloc(ncols * sizeof(int));
    for ( int h = 0; h < inst->nnodes; h++ ) 
	{
		int mycomp = comp[h];
		if ( sum_demand[mycomp] < -1.0 ) continue; // means that the component was already considered    
		int min_n_veh = (sum_demand[mycomp] / inst->capacity + 0.999);  
		sum_demand[mycomp] = -2.0;
		if ( nedges[mycomp] >= 2 * min_n_veh ) continue; 
		if  ( VERBOSE >= 100 ) printf("        ... violated SEC with n. edges %3d < %3d\n", nedges[mycomp], 2*min_n_veh);
		
		// add the violated SEC
		nnz = 0;
		rhs = -min_n_veh;
		for ( int i = 0; i < inst->nnodes; i++ ) 
		{                                             
			if ( comp[i] != mycomp ) continue;  
			rhs++;
			for ( int j = 0; j < inst->nnodes; j++ )
			{ 
				if ( comp[j] != mycomp || i >= j ) continue;  
				index[nnz] = zpos(i,j, inst);
				value[nnz] = 1.0;
				nnz++;
			}  
		}
		ncuts++;  
#if 0
	mip_add_cut(env, lp, -1, nnz, rhs, sense, index, value, -1);   			// -1 means static cuts
#else
	mip_add_cut(env, lp, -1, nnz, rhs, sense, index, value, -2);   			// -2 means lazy cuts
	mip_add_cut(env, lp, -1, nnz, rhs, sense, index, value, -3);   			// -3 means user cuts
#endif
	}           
	
	free(index);
	free(value);
	free(sum_demand);
	free(nedges);
	free(comp);
	free(xstar); 
	return ncuts;
}

/***************************************************************************************************************************/
void vrp_do_old_benders(instance *inst, CPXENVptr env, CPXLPptr lp)
/**************************************************************************************************************************/
{
	
	if ( VERBOSE >= 10 ) printf("\n\n ################# APPLYING OLD BENDERS #####################\n\n");
	CPXsetintparam(env, CPX_PARAM_SCRIND, (VERBOSE >= 100) ); 
	
	mip_set_level_for_all_cuts(env, 2);   

	int heu = 1; CPXsetdblparam(env, CPX_PARAM_EPGAP, 1e-3); //CPXsetintparam(env, CPX_PARAM_INTSOLLIM, 1); //CPXsetintparam(env, CPX_PARAM_NODELIM, 10);
	    	
	int newcuts = 1, iter = 0, ntot_cuts = 0;
	while ( newcuts )
	{
		if ( time_limit_expired(inst) ) break; 
		mip_timelimit(env, CPX_INFBOUND, inst);

		double t1 = second();
		CPXmipopt(env,lp);  
		double lb = mip_value(env,lp); 
		int nn = CPXgetnodecnt(env, lp);  
		newcuts = add_benders_cuts(inst, env, lp);   
		ntot_cuts += newcuts;
		if ( !newcuts && heu ) { heu = 0; newcuts = -1; CPXsetdblparam(env, CPX_PARAM_EPGAP, 1e-6); } //CPXsetintparam(env, CPX_PARAM_NODELIM, 999999999); CPXsetintparam(env, CPX_PARAM_INTSOLLIM, 99999999); }    
		
		if ( VERBOSE >= 20 ) printf(" ... old Benders: iter. %3d lower bound %15.5lf in %10.3lf sec.s (tot. %10.3lf sec.s) new cuts %4d [nodes %4d heu %d]\n", ++iter, lb, second()-t1, second()-inst->tstart, newcuts, nn, heu);
		
	}
	if ( VERBOSE >= 10 ) printf("\n ... old Benders: done after %4d iter.s (%5d cuts) in %10.3lf sec.s\n\n ################# OLD BENDERS DONE #########################\n\n", iter, ntot_cuts, second()-inst->tstart);
}


