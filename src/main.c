/* TOFO ggc -o3 or -o4 -o bin/main src/main && bin/main */
#include "vrp.h"
#include "greedy.h"
#include "parser.h"
#include "logger.h"
#include "plot.h"
#include "utils.h"
#include "heuristics.h"
#include "metaheuristic.h"
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
#ifndef PRODUCTION
	log_distancematrix(distance_matrix, args.nnodes);
#endif
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
	print_arguments(&args);
	srand(args.randomseed);
	Instance instances[args.num_instances];
	INFO_COMMENT("main::main", "Generating instances");
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
		instances[c_inst].grasp_n_probabilities = args.n_probabilities;
		instances[c_inst].grasp_probabilities = args.grasp_probabilities;
		strcpy(instances[c_inst].input_file, args.input_file);
		int *path = malloc(sizeof(int) * args.nnodes);
		generate_path(path, starting_points[c_inst], args.nnodes);

		char str_startingNode[20];
		sprintf(str_startingNode, "%d-", starting_points[c_inst]);

		char title[40] = "\0";

		for (int j = 0; j < n_passagges; j++)
		{
			INFO_COMMENT("main::main", "Generating instance %s", passagges[j]);
			if (strcmp(passagges[j], "nn") == 0)
			{
				nearest_neighboor(distance_matrix, path, instances[c_inst].nnodes, &instances[c_inst].tour_lenght);
			}
			else if (strcmp(passagges[j], "nng") == 0)
			{
#ifndef PRODUCTION
				log_path(path, instances[c_inst].nnodes);
#endif
				// parse_grasp_probabilities(args.grasp, instances[c_inst].grasp_probabilities, &instances[c_inst].grasp_n_probabilities);
				nearest_neighboor_grasp(distance_matrix, path, instances[c_inst].nnodes, &(instances[c_inst].tour_lenght), instances[c_inst].grasp_probabilities, instances[c_inst].grasp_n_probabilities);
			}
			else if (strcmp(passagges[j], "em") == 0)
			{
				extra_mileage(distance_matrix, path, instances[c_inst].nnodes, &instances[c_inst].tour_lenght);
			}
			else if (strcmp(passagges[j], "2opt") == 0)
			{
				two_opt(distance_matrix, instances[c_inst].nnodes, path, &(instances[c_inst].tour_lenght), INFINITY);
			}
			else if (strcmp(passagges[j], "vpn") == 0)
			{
				vnp_k(distance_matrix, path, instances[c_inst].nnodes, &instances[c_inst].tour_lenght, 5, 4);
			}
			else if (strcmp(passagges[j], "sa") == 0)
			{
				simulate_anealling(distance_matrix, path, instances[c_inst].nnodes, &instances[c_inst].tour_lenght, 10000, 100);
			}
			else if (strcmp(passagges[j], "tabu") == 0)
			{
				if (j == 0)
				{
					FATAL_COMMENT("main::main", "Tabu search must be used with a starting point");
				}
				tabu_search(distance_matrix, path, instances[c_inst].nnodes, &instances[c_inst].tour_lenght, args.nnodes / 10, 2);
			}
			else if (strcmp(passagges[j], "test") == 0)
			{
				continue;
			}
			else
			{
				FATAL_COMMENT("main::main", "Model %s not recognized", passagges[j]);
			}
			printf("finished");
			ffflush();
			strcpy(title + strlen(title), passagges[j]);
			plot(path, args.x, args.y, args.nnodes, title, instances[c_inst].node_start);
			if (j == n_passagges - 1)
				strcpy(title, "\0");
		}
		sprintf(title + strlen(title), str_startingNode);
		OUTPUT_COMMENT("main::main", "Tour lenght: %lf", instances[c_inst].tour_lenght);
		free(path);
	}
	free(distance_matrix);
}