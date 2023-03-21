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
	int number_of_instances = 5;
	logger_init("example.log");
	Instance inst;

	// Parsing Arguments ------------------------------------------------------
	INFO_COMMENT("main::parse_command_line", "Parsing arguments");
	Arguments args;
	parse_command_line(argc, argv, &args);
	inst = args.inst;

	if (args.help)
    {
        usage(stdout, 0);
        INFO_COMMENT("main::args.help","Help message displayed");
    }

	if (args.help_models)
	{
		show_models(stdout, 0);
		INFO_COMMENT("main::args.show_models","Show models message displayed");
	}

	INFO_COMMENT("main::print_arguments", "Printing arguments");
	print_arguments(stdout, &inst);
	

	// Read TLP library -------------------------------------------------------
	INFO_COMMENT("main::read_input","Reading input");
	read_input(&inst);
	DEBUG_COMMENT("main::read_input","Input read, nnodes= %d", inst.nnodes);
	
	// Manage model selection -------------------------------------------------
	switch (inst.model_type)
	{
	case 1:
		INFO_COMMENT("main", "Selected model nearest_neighboor"); 
		model_nearest_neighboor(&inst, number_of_instances); 
		break;
	 case 2:	
	 	INFO_COMMENT("main", "Selected model extra_mileage");
		//TODO implement starting points
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

	OUTPUT_COMMENT("main", "End of the program");
	plot(&inst);

	return 0;
}