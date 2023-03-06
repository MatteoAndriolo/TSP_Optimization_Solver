#include "vrp.h"

void parse_command_line(int argc, char **argv, Instance *inst);

void debug(const char *err)
{
	printf("\nDEBUG: %s \n", err);
	fflush(NULL);
}

void print_error(const char *err)
{
	printf("\n\n ERROR: %s \n\n", err);
	fflush(NULL);
	exit(1);
}
void read_input(Instance *);
void plot(Instance *);

int main(int argc, char **argv)
{
	Instance inst;
	if (argc < 2)
	{
		printf("Usage: %s -help for help\n", argv[0]);
		exit(1);
	}

	//double t1 = second();

	parse_command_line(argc, argv, &inst);
	if (inst.verbosity >= 2)
	{
		for (int a = 0; a < argc; a++)
			printf("%s ", argv[a]);
		printf("\n");
	}
	// printf(" file %s has %d non-empty lines\n", inst.input_file, number_of_nonempty_lines(inst.input_file)); exit(1);

	read_input(&inst);
	//if (VRPopt(&inst))
	//	print_error(" error within VRPopt()");
	//double t2 = second();

	//if (inst.verbosity >= 1)
	//{
		//printf("... VRP problem solved in %lf sec.s\n", t2 - t1);
	//}

	return 0;
}

//TODO aggiungi verbose per avere più log e qualche check interessante 
void read_input(Instance *inst) // simplified CVRP parser, not all SECTIONs detected
{

	FILE * fin = fopen(inst->input_file, "r");
	if (fin == NULL)
		print_error(" input file not found!");

	inst->nnodes = -1;
	
	char line[1024];
    int num_nodes = 0;
	int num_nodes_check = 0;
    int reading_nodes = 0;
    double node_x, node_y;
	int node_num;
	int i = 0;

    while (fgets(line, 1024, fin)) {
        // Strip trailing newline
        int len = strlen(line);
        if (line[len-1] == '\n') {
            line[len-1] = '\0';
        }

		if (sscanf(line, "DIMENSION : %d", &num_nodes) == 1){
				printf("Number of nodes: %d \n", num_nodes);
				num_nodes_check = num_nodes;
				
			}

        if (reading_nodes) {
			/*
			From each line take the node_num, allocate after a fixed memory
			with calloc of size of nom_nodes, store inside pair the 2 coordinates
			*/
            if (sscanf(line, "%d %lf %lf", &node_num, &node_x, &node_y) == 3) { 	  
				
				printf("Node %d: (%lf, %lf)\n", node_num, node_x , node_y);
				
				Pair* points = (Pair *) malloc(sizeof(Pair));
				i ++;
				points[i-1].x = node_x;
				points[i-1].y = node_y;
				inst->coord = points;
				
                
			} else {
                // Reached end of node coordinates section
                break;
            }
        } else {

            if (strcmp(line, "NODE_COORD_SECTION") == 0) {
                reading_nodes = 1;
            } else if (sscanf(line, "DIMENSION : %d", &num_nodes) == 1) ;
        }

    }
	/*
	Check if the actual number of the nodes defined in the file is different 
	from the real one
	*/
	if((i ) != num_nodes_check){
		print_error("... NUMBER_NODES differes from the actual number of nodes in the file");
	}

	for (int i = 0; i < num_nodes / 2; i++) {
    printf("Point %d: (%lf, %lf)\n", i, inst->coord[i].x, inst->coord[i].y);
	}

    printf("Parsed %d nodes.\n", num_nodes / 2);

    fclose(fin);
}


void parse_command_line(int argc, char **argv, Instance *inst)
{
	int help = 0;
	// default
	inst->model_type = 0;
	strcpy(inst->input_file, "NULL");
	inst->randomseed = 0;
	inst->num_threads = 0;
	inst->timelimit = INFTY;
	inst->cutoff = INFTY;
	//inst->timelimit = CPX_INFBOUND;
	//inst->cutoff = CPX_INFBOUND;
	inst->integer_costs = 0;
	inst->available_memory = 12000; // available memory, in MB, for Cplex execution (e.g., 12000)
	inst->max_nodes = -1;			// max n. of branching nodes in the final run (-1 unlimited)

	static struct option long_options[] = {
		{"file", required_argument, 0, 'f'},
		{"input", required_argument, 0, 'f'},
		{"model", required_argument, 0, 'm'},
		{"model_type", required_argument, 0, 'M'},
		{"seed", required_argument, 0, 's'},
		{"threads", required_argument, 0, 't'},
		{"time_limit", required_argument, 0, 'l'},
		{"memory", required_argument, 0, 'L'},
		{"node_file", required_argument, 0, 'n'},
		{"max_nodes", required_argument, 0, 'N'},
		{"cutoff", required_argument, 0, 'c'},
		{"int", required_argument, 0, 'i'},
		{"help", required_argument, 0, 'h'},
		{0, 0, 0, 0}};
	int opt;
	while ((opt = getopt(argc, argv, "f:m:M:s:h:l:L:n:N:c:i:t")) != -1)
	{
		switch (opt)
		{
			case 'f':
			 	//TODO debug input file, probably cuts the adress
				strcpy(inst->input_file, optarg);
				break; // input file
			case 'l':
				inst->timelimit = atof(optarg);
				break; // total time limit
			case 'M':
				inst->model_type = atoi(optarg);
				break; // model type
			case 'm':
				inst->model_type = atoi(optarg);
				break; // model type
			case 's':
				inst->randomseed = abs(atoi(optarg));
				break; // random seed
			case 't':
				inst->num_threads = atoi(optarg);
				break; // n. threads
			case 'L':
				inst->available_memory = atoi(optarg);
				break; // available memory (in MB)
			case 'n':
				strcpy(inst->node_file, optarg);
				break; // cplex's node file
			case 'N':
				inst->max_nodes = atoi(optarg);
				break; // max n. of nodes
			case 'c':
				inst->cutoff = atof(optarg);
				break; // master cutoff
			case 'i':
				inst->integer_costs = 1;
				break; // inteher costs
			case 'h':
				help = 1;
				break; // help
			default:
				printf("Invalid option\n");
		}
	}

	if (inst->verbosity >= 100)
		printf(" running %s with %d parameters \n", argv[0], argc - 1);

	if (argc < 1)
		help = 1;

	if (help || (inst->verbosity >= 1)) // print current parameters
	{
		printf("\n\navailable parameters (vers. 16-may-2015) --------------------------------------------------\n");
		printf("-f -file %s\n", inst->input_file);
		printf("-l -time_limit %lf\n", inst->timelimit);
		printf("-m -model_type %d\n", inst->model_type);
		printf("-s -seed %d\n", inst->randomseed);
		printf("-t -threads %d\n", inst->num_threads);
		printf("-N -max_nodes %d\n", inst->max_nodes);
		printf("-L -memory %d\n", inst->available_memory);
		printf("-i -int %d\n", inst->integer_costs);
		printf("-n -node_file %s\n", inst->node_file);
		printf("-c -cutoff %lf\n", inst->cutoff);
		printf("\nenter -help or --help for help\n");
		printf("----------------------------------------------------------------------------------------------\n\n");
	}

	if (help)
		exit(1);
}

void plot(Instance *inst){
	return;
}