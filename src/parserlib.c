#include "lib/parserlib.h"

void read_input(Instance *inst) // simplified CVRP parser, not all SECTIONs detected
{
	if(inst->verbosity >= 4){
		printf("... START reading the file");
	}

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
				if (inst->verbosity >= 3) printf("Number of nodes in the field DIMENTION: %d \n", num_nodes);
				inst->nnodes = num_nodes;
				inst->x = (double* ) calloc(num_nodes, sizeof(double));
				inst->y = (double* ) calloc(num_nodes, sizeof(double));
				if (inst->verbosity >= 4) printf("Memory for x and y allocated\n");
			}

		

        if (reading_nodes) {
			/*
			From each line take the node_num, allocate after a fixed memory
			with calloc of size of nom_nodes, store inside pair the 2 coordinates
			*/
            if (sscanf(line, "%d %lf %lf", &node_num, &node_x, &node_y) == 3) { 	  
				
				if (inst->verbosity>=4)("Node %d: (%lf, %lf)\n", node_num, node_x , node_y);
        		inst->x[i++] = node_x; //TODO replace i with node_num node could be placed in different location
				inst->y[i++] = node_y;
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
	

	if(inst->verbosity >= 3){
	for (int i = 0; i < num_nodes_check; i++) {
     	printf("Point %d: (%lf, %lf)\n", i, inst->x[i] , inst->y[i]);
	}

	if (inst->verbosity >= 3) printf("Parsed %d nodes.\n", num_nodes);
	}
    
	if(inst->verbosity >= 2) printf("... ENDING reading the file");
    fclose(fin);
}
