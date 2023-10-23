/* TOFO ggc -o3 or -o4 -o bin/main src/main && bin/main */
#include "vrp.h"
#include "greedy.h"
#include "parser.h"
#include "logger.h"
#include "plot.h"
#include "utils.h"
#include "heuristics.h"
#include "tspcplex.h"
#include "simulate_anealling.h"
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	logger_init("example.log");

	// Parsing Arguments ------------------------------------------------------
	INFO_COMMENT("main::parse_command_line", "Parsing arguments");
	Args args;
	parse_command_line(argc, argv, &args);

	print_arguments(&args);
	// Read TLP library -------------------------------------------------------
	INFO_COMMENT("main::main", "Reading input");
	read_input(&args);
	DEBUG_COMMENT("main::main", "Input read, nnodes= %d", args.nnodes);

	// Generate distance matrix -----------------------------------------------
	INFO_COMMENT("main::main", "Generating distance matrix");
	double *distance_matrix = (double *)malloc(sizeof(double) * args.nnodes * args.nnodes);
	generate_distance_matrix(&distance_matrix, args.nnodes, args.x, args.y, args.integer_costs);
	// log_distancematrix(distance_matrix, args.nnodes);
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
	for (int i = 0; i < args.num_instances; i++)
	{
		DEBUG_COMMENT("main::main", "Starting point %d: %d", i, starting_points[i]);
	}

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
		char title[40] = "\0";
		snprintf(title, 25, "sn%d_", starting_points[c_inst]);
		for (int j = 0; j < n_passagges; j++)
		{
			printf("main::main | Generating instance");
			INFO_COMMENT("main::main", "Generating instance");
			INFO_COMMENT("main.c::main", "model %s", passagges[j]);
			if (strcmp(passagges[j], "nn") == 0)
			{
				nearest_neighboor(distance_matrix, path, args.nnodes, &instances[c_inst].tour_lenght);
			}
			if (strcmp(passagges[j], "nng") == 0)
			{
				int n_prob;
				double *prob;
				parse_grasp_probabilities(args.grasp, &prob, &n_prob);
				sprintf(title + strlen(title), "_%.1f_", prob[0]);
				for (int i = 1; i < n_prob; i++)
				{
					sprintf(title + strlen(title), "_%.1f_", prob[i] - prob[i - 1]);
				}
				nearest_neighboor_grasp(distance_matrix, path, args.nnodes, &(instances[c_inst].tour_lenght), prob, n_prob);
			}
			if (strcmp(passagges[j], "em") == 0)
			{
				extra_mileage(distance_matrix, path, args.nnodes, &(instances[c_inst].tour_lenght));
			}
			if (strcmp(passagges[j], "2opt") == 0)
			{
				two_opt(distance_matrix, args.nnodes, path, &(instances[c_inst].tour_lenght));
			}
			if (strcmp(passagges[j], "vpn") == 0)
			{
				vnp_k(distance_matrix, path, args.nnodes, &instances[c_inst].tour_lenght, 5, 4);
			}
			if (strcmp(passagges[j], "cplex") == 0)
			{
				TSPopt(&instances[c_inst], path);
			}
			if (strcmp(passagges[j], "cplexheu") == 0)
			{
				instances[c_inst].percentageHF = 80; // TODO implement in cmd argument parsing
				instances[c_inst].solver = 4;
				two_opt(distance_matrix, args.nnodes, path, &(instances[c_inst].tour_lenght));
				TSPopt(&instances[c_inst], path);
			}
			if (strcmp(passagges[j], "sim_anealling") == 0)
			{
				simulate_anealling(distance_matrix, path, args.nnodes, &(instances[c_inst].tour_lenght), 100, 1000);
			}
			strcpy(title + strlen(title), passagges[j]);
			// TODO fix title in all the different
			//  like in grasp specify also the probabilities
			//  put first name of model then the rest
			plot(path, args.x, args.y, args.nnodes, title, instances[c_inst].node_start);
			if (j == n_passagges - 1)
				strcpy(title, "\0");
		}
		free(path);
	}

	OUTPUT_COMMENT("main", "End of the program");
	return 0;
}