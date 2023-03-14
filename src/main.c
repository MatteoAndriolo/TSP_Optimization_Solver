/* TOFO ggc -o3 or -o4 -o bin/main src/main && bin/main */
#include "vrp.h"
#include "greedy.h"
#include "parser.h"
#include "logger.h"
#include "plot.h"
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	logger_init("example.log");
	Instance inst;

	// Parsing Arguments ------------------------------------------------------
	INFO_COMMENT("main::parse_command_line", "Parsing arguments");
	struct arguments args;
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

	DEBUG_COMMENT("main::read_input","Reading input %d", inst.nnodes);
	
	// Plot -------------------------------------------------------------------
	//INFO_COMMENT("main::plot", "Plotting with gnuplot"); 
	//plot(&inst);

	switch (inst.model_type)
	{
	case 1:
		INFO_COMMENT("main", "Start model nearest_neighboor"); 
		model_nearest_neighboor(&inst); 
		break;
	case 2:	
		INFO_COMMENT("main", "Start model extra_mileage");
		extra_mileage(&inst);
		break;
	case 3:
		INFO_COMMENT("main", "Start model modified_extra_mileage");
		updated_extra_mileage(&inst);
		break;
	
	default:
		ERROR_COMMENT("main", "Model not found");
		show_models(stdout, 0);
		break;
	}

	return 0;
}