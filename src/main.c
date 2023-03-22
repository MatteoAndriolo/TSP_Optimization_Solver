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
	// Instance inst[args.num_instances];

	//	if (args.help)
	//	{
	//		usage(stdout, 0);
	//		INFO_COMMENT("main::args.help", "Help message displayed");
	//	}

	//	if (args.help_models)
	//	{	// 	matrix = generate_distance_matrix(matrix, inst.nnodes, path, path, 2);

	//		show_models(stdout, 0);
	//		INFO_COMMENT("main::args.show_models", "Show models message displayed");
	//	}

	INFO_COMMENT("main::main", "Printing arguments");
	//print_arguments(&args);

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
	parse_model_name(args.model_type,  &passagges,  &n_passagges);
	for(int i=0;i<n_passagges;i++){
		printf("%s\n",passagges[i]);
	}


	// Generate instance ------------------------------------------------------
	INFO_COMMENT("main::main", "Generating instance");
	//Instance inst[args.num_instances];

	// Generate starting points -----------------------------------------------
	INFO_COMMENT("main::main", "Generating starting points");
	int *starting_points = (int *)malloc(sizeof(int) * args.num_instances);
	generate_random_starting_nodes(starting_points, args.nnodes, args.num_instances, args.randomseed);
	/*
	// Manage model selection -------------------------------------------------
	for (int i = 0; i < n_passagges + 1; i++)
	{

		switch (args.model_type)
		{
		case 1:
			INFO_COMMENT("main", "Selected model nearest_neighboor");
			model_nearest_neighboor(&inst, number_of_instances);
			break;
		case 2:
			INFO_COMMENT("main", "Selected model extra_mileage");
			// TODO implement starting points
			extra_mileage(&inst, number_of_instances);
			break;
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
		default:
			ERROR_COMMENT("main", "Model not found");
			show_models(stdout, 0);
			break;
		}
	}

	OUTPUT_COMMENT("main", "End of the program");
	return 0;
	*/
}