#include "vrp.h"
#include "log.h"
#include "parser.h"
#include "plot.h"
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	Instance inst;

	// Parsing Arguments
	struct arguments args;
	parse_command_line(argc, argv, &args);
	inst = args.inst;

	if (args.help)
		usage(stdout, 0);
	if (inst.verbosity > 1)
		print_arguments(stdout, &inst);

	// Read TLP library
	read_input(&inst);
	if (inst.verbosity > 1)
		("Number nodes %d", inst.nnodes);
	
	// Plot 
	plot_solution(&inst);

	return 0;
}