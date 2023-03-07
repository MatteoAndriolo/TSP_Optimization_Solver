#include "vrp.h"           

double second();
void print_error(const char *err);  
void check_dblsort();       
double random01();     
void read_input(instance *inst);
void parse_command_line(int argc, char** argv, instance *inst); 

void debug(const char *err) { printf("\nDEBUG: %s \n", err); fflush(NULL); }
void print_error(const char *err) { printf("\n\n ERROR: %s \n\n", err); fflush(NULL); exit(1); }   

int number_of_nonempty_lines(const char *file)  // warning: the last line NOT counted if it is does not terminate with \n (as it happens with some editors) 
{
	FILE *fin = fopen(file, "r");
	if ( fin == NULL ) return 0;
	char line[123456]; 
	int count = 0;
	while( fgets(line, sizeof(line), fin) != NULL ) { printf(" len %4d\n", (int) strlen(line)); if ( strlen(line) > 1 ) count++; }
	fclose(fin);   
	return count; 	
}


void free_instance(instance *inst)
{     
	free(inst->demand);
	free(inst->xcoord);
	free(inst->ycoord);
	free(inst->load_min);
	free(inst->load_max);
}

int VRPopt(instance *inst);    

int main(int argc, char **argv) 
{ 
	if ( argc < 2 ) { printf("Usage: %s -help for help\n", argv[0]); exit(1); }       
	if ( VERBOSE >= 2 ) { for (int a = 0; a < argc; a++) printf("%s ", argv[a]); printf("\n"); }

	double t1 = second(); 
	instance inst;

	parse_command_line(argc,argv, &inst);     
	
	//printf(" file %s has %d non-empty lines\n", inst.input_file, number_of_nonempty_lines(inst.input_file)); exit(1);
	  
	read_input(&inst);  
	if ( VRPopt(&inst) ) print_error(" error within VRPopt()");
	double t2 = second(); 
    
	if ( VERBOSE >= 1 )   
	{
		printf("... VRP problem solved in %lf sec.s\n", t2-t1);  
	}
	
	free_instance(&inst);
	return 0; 
}         

void read_input(instance *inst) // simplified CVRP parser, not all SECTIONs detected  
{
                            
	FILE *fin = fopen(inst->input_file, "r");
	if ( fin == NULL ) print_error(" input file not found!");
	
	inst->nnodes = -1;
	inst->depot = -1;  
	inst->nveh = -1;

	char line[180];
	char *par_name;   
	char *token1;
	char *token2;
	
	int active_section = 0; // =1 NODE_COORD_SECTION, =2 DEMAND_SECTION, =3 DEPOT_SECTION 
	
	int do_print = ( VERBOSE >= 1000 );

	while ( fgets(line, sizeof(line), fin) != NULL ) 
	{
		if ( VERBOSE >= 2000 ) { printf("%s",line); fflush(NULL); }
		if ( strlen(line) <= 1 ) continue; // skip empty lines
	    par_name = strtok(line, " :");
		if ( VERBOSE >= 3000 ) { printf("parameter \"%s\" ",par_name); fflush(NULL); }

		if ( strncmp(par_name, "NAME", 4) == 0 ) 
		{
			active_section = 0;
			continue;
		}

		if ( strncmp(par_name, "COMMENT", 7) == 0 ) 
		{
			active_section = 0;   
			token1 = strtok(NULL, "");  
			if ( VERBOSE >= 10 ) printf(" ... solving instance %s with model %d\n\n", token1, inst->model_type);
			continue;
		}   
		
		if ( strncmp(par_name, "TYPE", 4) == 0 ) 
		{
			token1 = strtok(NULL, " :");  
			if ( strncmp(token1, "CVRP",4) != 0 ) print_error(" format error:  only TYPE == CVRP implemented so far!!!!!!"); 
			active_section = 0;
			continue;
		}
		

		if ( strncmp(par_name, "DIMENSION", 9) == 0 ) 
		{
			if ( inst->nnodes >= 0 ) print_error(" repeated DIMENSION section in input file");
			token1 = strtok(NULL, " :");
			inst->nnodes = atoi(token1);
			if ( do_print ) printf(" ... nnodes %d\n", inst->nnodes); 
			inst->demand = (double *) calloc(inst->nnodes, sizeof(double)); 	 
			inst->xcoord = (double *) calloc(inst->nnodes, sizeof(double)); 	 
			inst->ycoord = (double *) calloc(inst->nnodes, sizeof(double));    
			active_section = 0;  
			continue;
		}

		if ( strncmp(par_name, "CAPACITY", 8) == 0 ) 
		{
			token1 = strtok(NULL, " :");
			inst->capacity = atof(token1);
			if ( do_print ) printf(" ... vehicle capacity %lf\n", inst->capacity); 
			active_section = 0;
			continue;
		}


		if ( strncmp(par_name, "VEHICLES", 8) == 0 ) 
		{
			token1 = strtok(NULL, " :");
			inst->nveh = atoi(token1);
			if ( do_print ) printf(" ... n. vehicles %d\n", inst->nveh);  
			active_section = 0;
			continue;
		}


		if ( strncmp(par_name, "EDGE_WEIGHT_TYPE", 16) == 0 ) 
		{
			token1 = strtok(NULL, " :");
			if ( strncmp(token1, "EUC_2D", 6) != 0 ) print_error(" format error:  only EDGE_WEIGHT_TYPE == EUC_2D implemented so far!!!!!!"); 
			active_section = 0;
			continue;
		}            
		
		if ( strncmp(par_name, "NODE_COORD_SECTION", 18) == 0 ) 
		{
			if ( inst->nnodes <= 0 ) print_error(" ... DIMENSION section should appear before NODE_COORD_SECTION section");
			active_section = 1;   
			continue;
		}
		
		if ( strncmp(par_name, "DEMAND_SECTION", 14) == 0 ) 
		{
			if ( inst->nnodes <= 0 ) print_error(" ... DIMENSION section should appear before DEMAND_SECTION section");
			active_section = 2;
			continue;
		}  

		if ( strncmp(par_name, "DEPOT_SECTION", 13) == 0 )  
		{
			if ( inst->depot >= 0 ) print_error(" ... DEPOT_SECTION repeated??");
			active_section = 3;   
			continue;
		}

		
		if ( strncmp(par_name, "EOF", 3) == 0 ) 
		{
			active_section = 0;
			break;
		}
		
			
		if ( active_section == 1 ) // within NODE_COORD_SECTION
		{
			int i = atoi(par_name) - 1; 
			if ( i < 0 || i >= inst->nnodes ) print_error(" ... unknown node in NODE_COORD_SECTION section");     
			token1 = strtok(NULL, " :,");
			token2 = strtok(NULL, " :,");
			inst->xcoord[i] = atof(token1);
			inst->ycoord[i] = atof(token2);
			if ( do_print ) printf(" ... node %4d at coordinates ( %15.7lf , %15.7lf )\n", i+1, inst->xcoord[i], inst->ycoord[i]); 
			continue;
		}    
		  
		if ( active_section == 2 ) // within DEMAND_SECTION
		{
			int i = atoi(par_name) - 1; 
			if ( i < 0 || i >= inst->nnodes ) print_error(" ... unknown node in NODE_COORD_SECTION section");     
			token1 = strtok(NULL, " :,");
			inst->demand[i] = atof(token1);
			if ( do_print ) printf(" ... node %4d has demand %10.5lf\n", i+1, inst->demand[i]); 
			continue;
		}  

		if ( active_section == 3 ) // within DEPOT_SECTION
		{
			int i = atoi(par_name) - 1; 
			if ( i < 0 || i >= inst->nnodes ) continue;
			if ( inst->depot >= 0 ) print_error(" ... multiple depots not supported in DEPOT_SECTION");     
			inst->depot = i;
			if ( do_print ) printf(" ... depot node %d\n", inst->depot+1); 
			continue;
		}  
		
		printf(" final active section %d\n", active_section);
		print_error(" ... wrong format for the current simplified parser!!!!!!!!!");     
		    
	}                

	fclose(fin);    
	
}

void parse_command_line(int argc, char** argv, instance *inst) 
{ 
	
	if ( VERBOSE >= 100 ) printf(" running %s with %d parameters \n", argv[0], argc-1); 
		
	// default   
	inst->model_type = 0;
	inst->old_benders = 0;
	strcpy(inst->input_file, "NULL");
	inst->randomseed = 0; 
	inst->num_threads = 0;
	inst->timelimit = CPX_INFBOUND;
	inst->cutoff = CPX_INFBOUND; 
	inst->integer_costs = 0;

	inst->available_memory = 12000;   			// available memory, in MB, for Cplex execution (e.g., 12000)
	inst->max_nodes = -1; 						// max n. of branching nodes in the final run (-1 unlimited)        

    int help = 0; if ( argc < 1 ) help = 1;	
	for ( int i = 1; i < argc; i++ ) 
	{ 
		if ( strcmp(argv[i],"-file") == 0 ) { strcpy(inst->input_file,argv[++i]); continue; } 			// input file
		if ( strcmp(argv[i],"-input") == 0 ) { strcpy(inst->input_file,argv[++i]); continue; } 			// input file
		if ( strcmp(argv[i],"-f") == 0 ) { strcpy(inst->input_file,argv[++i]); continue; } 				// input file
		if ( strcmp(argv[i],"-time_limit") == 0 ) { inst->timelimit = atof(argv[++i]); continue; }		// total time limit
		if ( strcmp(argv[i],"-model_type") == 0 ) { inst->model_type = atoi(argv[++i]); continue; } 	// model type
		if ( strcmp(argv[i],"-old_benders") == 0 ) { inst->old_benders = atoi(argv[++i]); continue; } 	// old benders
		if ( strcmp(argv[i],"-model") == 0 ) { inst->model_type = atoi(argv[++i]); continue; } 			// model type
		if ( strcmp(argv[i],"-seed") == 0 ) { inst->randomseed = abs(atoi(argv[++i])); continue; } 		// random seed
		if ( strcmp(argv[i],"-threads") == 0 ) { inst->num_threads = atoi(argv[++i]); continue; } 		// n. threads
		if ( strcmp(argv[i],"-memory") == 0 ) { inst->available_memory = atoi(argv[++i]); continue; }	// available memory (in MB)
		if ( strcmp(argv[i],"-node_file") == 0 ) { strcpy(inst->node_file,argv[++i]); continue; }		// cplex's node file
		if ( strcmp(argv[i],"-max_nodes") == 0 ) { inst->max_nodes = atoi(argv[++i]); continue; } 		// max n. of nodes
		if ( strcmp(argv[i],"-cutoff") == 0 ) { inst->cutoff = atof(argv[++i]); continue; }				// master cutoff
		if ( strcmp(argv[i],"-int") == 0 ) { inst->integer_costs = 1; continue; } 						// inteher costs
		if ( strcmp(argv[i],"-help") == 0 ) { help = 1; continue; } 									// help
		if ( strcmp(argv[i],"--help") == 0 ) { help = 1; continue; } 									// help
		help = 1;
    }      

	if ( help || (VERBOSE >= 10) )		// print current parameters
	{
		printf("\n\navailable parameters (vers. 16-may-2015) --------------------------------------------------\n");
		printf("-file %s\n", inst->input_file); 
		printf("-time_limit %lf\n", inst->timelimit); 
		printf("-model_type %d\n", inst->model_type); 
		printf("-old_benders %d\n", inst->old_benders); 
		printf("-seed %d\n", inst->randomseed); 
		printf("-threads %d\n", inst->num_threads);  
		printf("-max_nodes %d\n", inst->max_nodes); 
		printf("-memory %d\n", inst->available_memory); 
		printf("-int %d\n", inst->integer_costs); 
		printf("-node_file %s\n", inst->node_file);
		printf("-cutoff %lf\n", inst->cutoff); 
		printf("\nenter -help or --help for help\n");
		printf("----------------------------------------------------------------------------------------------\n\n");
	}        
	
	if ( help ) exit(1);

}    





