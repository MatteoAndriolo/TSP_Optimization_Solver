/* TOFO ggc -o3 or -o4 -o bin/main src/main && bin/main */
#include "vrp.h"
#include "greedy.h"
#include "parser.h"
#include "logger.h"
#include "plot.h"
#include "utils.h"
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	logger_init("example.log");

	// Parsing Arguments ------------------------------------------------------
	INFO_COMMENT("main::parse_command_line", "Parsing arguments");
	Args args;
	parse_command_line(argc, argv, &args);

	// Read TLP library -------------------------------------------------------
	INFO_COMMENT("main::main", "Reading input");
	read_input(&args);
	DEBUG_COMMENT("main::main", "Input read, nnodes= %d", args.nnodes);

	// Generate distance matrix -----------------------------------------------
	INFO_COMMENT("main::main", "Generating distance matrix");
	double *distance_matrix = (double *)malloc(sizeof(double) * args.nnodes * args.nnodes);
	generate_distance_matrix(&distance_matrix, args.nnodes, args.x, args.y, args.integer_costs);
	DEBUG_COMMENT("main::main", "Distance matrix generated");

	// Parsing model ----------------------------------------------------------
	int n_passagges;
	char **passagges;
	parse_model_name(args.model_type, &passagges, &n_passagges);

	// Generate instance ------------------------------------------------------
	INFO_COMMENT("main::main", "Generating instance");
	// Instance inst[args.num_instances];

	// Generate starting points -----------------------------------------------
	INFO_COMMENT("main::main", "Generating starting points");
	int *starting_points = (int *)malloc(sizeof(int) * args.num_instances);
	generate_random_starting_nodes(starting_points, args.nnodes, args.num_instances, args.randomseed);

	// Manage model selection -------------------------------------------------
	Instance instances[args.num_instances];
	for (int c_inst = 0; c_inst < args.num_instances; c_inst++)
	{
		instances[c_inst].nnodes = args.nnodes;
		instances[c_inst].x = args.x;
		instances[c_inst].y = args.y;
		instances[c_inst].node_start = starting_points[c_inst];
		instances[c_inst].integer_costs = args.integer_costs;
		instances[c_inst].randomseed = args.randomseed;
		instances[c_inst].timelimit = args.timelimit;
		instances[c_inst].tour_lenght = 0;
		strcpy(instances[c_inst].input_file, args.input_file);

		int *path = malloc(sizeof(int) * args.nnodes);
		generate_path(path, starting_points[c_inst], args.nnodes);
		char title[25]="\0";
		snprintf(title, 25, "sn%d_", starting_points[c_inst]);
		for (int j = 0; j < n_passagges; j++)
		{
			INFO_COMMENT("main::main", "Generating instance");
			if (strcmp(passagges[j], "nn") == 0)
			{
				nearest_neighboor(distance_matrix, path, args.nnodes, &instances[c_inst].tour_lenght);
			}
			if (strcmp(passagges[j], "nng") == 0)
			{				
				int n_prob;
				double *prob;
				parse_grasp_probabilities(*args.grasp, &prob, &n_prob);
				for(int i=0;i<n_prob;i++)
				{
					printf("%lf",prob[i]);
				}
				//nearest_neighboor_grasp(distance_matrix, path, args.nnodes, &instances[c_inst].tour_lenght);
			}
			if (strcmp(passagges[j], "em")==0)
			{
				extra_mileage(distance_matrix, path, args.nnodes, &instances[c_inst].tour_lenght);
			}
			/*
			if(passagges[j] =="nng")
			{
				nearest_neighboor_grasp(distance_matrix,path, args.nnodes, instances[c_inst].zbest);
				continue;
			}
			*/
			if (strcmp(passagges[j], "2opt") == 0)
			{
				two_opt(distance_matrix, args.nnodes, path, &instances[c_inst].tour_lenght);
			}
			strcpy(title+strlen(title), passagges[j]);	
			plot(path, args.x,args.y, args.nnodes, title );
		}
		free(path);
	}
	// case 3:
	// 	INFO_COMMENT("main", "Selected model modified_extra_mileage");
	// 	updated_extra_mileage(&inst);
	// 	break;
	// case 4:
	// 	INFO_COMMENT("main", "generate all istance for eache algortihm");
	// 	double *matrix = (double *) malloc(sizeof(double) * inst.nnodes * inst.nnodes);
	// 	int *path = (int *) malloc(sizeof(int) * inst.nnodes);
	// 	//TODO: change the starting node
	// 	//TODO: fix rounding error
	// 	generate_path(path, 4, inst.nnodes);
	// 	matrix = generate_distance_matrix(matrix, inst.nnodes, path, path, 2);
	//	break;

	OUTPUT_COMMENT("main", "End of the program");
	return 0;
}