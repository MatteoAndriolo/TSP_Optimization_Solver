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
	
    // Log messages at various log levels
	Instance inst;

	// Parsing Arguments
	log_message(INFO, "main::parse_command_line", "Parsing arguments");
	struct arguments args;
	parse_command_line(argc, argv, &args);
	inst = args.inst;

	if (args.help)
    {
        usage(stdout, 0);
        log_message(INFO, "main::args.help","Help message displayed");
    }

	log_message(INFO, "main::print_arguments", "Printing arguments");
	print_arguments(stdout, &inst);
	

	// Read TLP library
	log_message(INFO, "main::read_input","Reading input");
	read_input(&inst);

	log_message(DEBUG, "main::read_input","Reading input %d", inst.nnodes);
	
	// Plot
	//log_message(INFO,"main::plot", "Plotting with gnuplot"); 
	//plot(&inst);

	log_message(INFO,"main", "Start model_nearest_neighboor"); 
	ffflush();
	model_nearest_neighboor(&inst);
	log_message(INFO,"main", "Start extra_mileage");
	ffflush();
	extra_mileage(&inst);

	return 0;
}