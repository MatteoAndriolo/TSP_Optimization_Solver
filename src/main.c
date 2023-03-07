#include "vrp.h"
#include "log.h"
#include "parser.h"
#include "plot.h"
#include<stdlib.h>
#include<unistd.h>

/*VERBOSITY
1) only the output
2) essencial passes
3) passes and some information
4) verbose almost all 
5) every think the log as well 
*/

int main(int argc, char **argv)
{
	Instance inst;

	// Parsing Arguments
	struct arguments args;
    parse_command_line(argc, argv, &args);
    inst=args.inst;

    if(args.help) usage(stdout, 0);
    if(args.inst.verbosity>1) print_arguments(stdout, &inst);
    
	//double t1 = second();

	// printf(" file %s has %d non-empty lines\n", inst.input_file, number_of_nonempty_lines(inst.input_file)); exit(1);

	read_input(&inst);
	//if (VRPopt(&inst))
	//	print_error(" error within VRPopt()");
	//double t2 = second();

	//if (inst.verbosity >= 1)
	//{
		//printf("... VRP problem solved in %lf sec.s\n", t2 - t1);
	//}
	printf("Number nodes %d", inst.nnodes);
	sleep(1);
	plot(&inst);
	return 0;
}


