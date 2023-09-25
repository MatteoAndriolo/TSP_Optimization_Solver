/* TOFO ggc -o3 or -o4 -o bin/main src/main && bin/main */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/greedy.h"
#include "../include/heuristics.h"
#include "../include/logger.h"
#include "../include/metaheuristic.h"
#include "../include/parser.h"
#include "../include/plot.h"
#include "../include/tabu.h"
#include "../include/utils.h"
#include "../include/vrp.h"

int main(int argc, char **argv) {
  logger_init("example.log");

  // Parsing Arguments ------------------------------------------------------
  Args args;
  parse_command_line(argc, argv, &args);

  // Read TLP library -------------------------------------------------------
  read_input(&args);

  // Generate distance matrix -----------------------------------------------
  double *distance_matrix =
      (double *)malloc(sizeof(double) * args.nnodes * args.nnodes);
  generate_distance_matrix(&distance_matrix, args.nnodes, args.x, args.y,
                           args.integer_costs);

  // Parsing model ----------------------------------------------------------
  int n_passagges;
  char **passagges;
  parse_model_name(args.model_type, &passagges, &n_passagges);

  // Generate instance ------------------------------------------------------
  // INFO_COMMENT("main::main", "Generating instance");
  // Instance inst[args.num_instances];

  // Generate starting points -----------------------------------------------
  int *starting_points = (int *)malloc(sizeof(int) * args.num_instances);
  generate_random_starting_nodes(starting_points, args.nnodes,
                                 args.num_instances, args.randomseed);

  // Manage model selection -------------------------------------------------
  print_arguments(&args);
  srand(args.randomseed);                  // set random seed
  Instance instances[args.num_instances];  // array of instances

  // RUN
  for (int c_inst = 0; c_inst < args.num_instances; c_inst++) {
    time_t tstart = time(NULL);
    //Instance myInstance;
    //initializeInstance(&myInstance);
    //modifyTourLengthSafely(&myInstance, 10.5);
    //destroyInstance(&myInstance);

    instances[c_inst].nnodes = args.nnodes;
    instances[c_inst].x = args.x;
    instances[c_inst].y = args.y;
    instances[c_inst].node_start = starting_points[c_inst];
    instances[c_inst].integer_costs = args.integer_costs;
    instances[c_inst].randomseed = args.randomseed;
    // save timelimit as now + seconds
    instances[c_inst].timelimit = time(NULL) + args.timelimit;
    instances[c_inst].tour_length = 0;
    instances[c_inst].grasp_n_probabilities = args.n_probabilities;
    instances[c_inst].grasp_probabilities = args.grasp_probabilities;
    strcpy(instances[c_inst].input_file, args.input_file);
    int *path = malloc(sizeof(int) * args.nnodes);
    generate_path(path, starting_points[c_inst], args.nnodes, true);

    char str_startingNode[20];
    sprintf(str_startingNode, "%d-", starting_points[c_inst]);

    char title[40] = "\0";

    for (int j = 0; j < n_passagges; j++) {
      INFO_COMMENT("main::main", "Generating instance %s", passagges[j]);
      printf("Generating instance %s\n", passagges[j]);
      // print strcmp(passagges[j], "nn");
      printf("strcmp %d\n", strcmp(passagges[j], "nn"));
      if (strcmp(passagges[j], "em") == 0) {
        extra_mileage(&instances[c_inst]);
      } else if (strcmp(passagges[j], "2opt") == 0) {
        two_opt(distance_matrix, instances[c_inst].nnodes, path,
                &(instances[c_inst].tour_length), INFINITY,
                instances[c_inst].timelimit);
      } else if (strcmp(passagges[j], "tabu") == 0) {
        if (j == 0) {
          FATAL_COMMENT("main::main",
                        "Tabu search must be used with a starting point");
        }
        tabu_search(distance_matrix, path, instances[c_inst].nnodes,
                    &(instances[c_inst].tour_length), (int)(args.nnodes / 20),
                    (int)(args.nnodes / 8), 2);
      } else if (strcmp(passagges[j], "vns") == 0) {
        ERROR_COMMENT("main::main", "VNS not implemented yet");
      } else if (strcmp(passagges[j], "sa") == 0) {
        simulate_anealling(distance_matrix, path, instances[c_inst].nnodes,
                           &instances[c_inst].tour_length, 10000, 100);
      } else if (strcmp(passagges[j], "gen") == 0) {
        ERROR_COMMENT("main::main", "Genetic algorithm not implemented yet");
      } else if (strcmp(passagges[j], "nn") == 0) {
        printf("nn\n");
        nearest_neighboor(&instances[c_inst]);
        printf("tourlenght %lf\n", instances[c_inst].tour_length);
      } else if (strcmp(passagges[j], "nng") == 0) {
        nearest_neighboor_grasp(&instances[c_inst]);
      } else if (strcmp(passagges[j], "vpn") == 0) {
        vnp_k(distance_matrix, path, instances[c_inst].nnodes,
              &instances[c_inst].tour_length, 5, 4);
      } else if (strcmp(passagges[j], "test") == 0) {
        test_buffer();
      } else {
        FATAL_COMMENT("main::main", "Model %s not recognized", passagges[j]);
      }
      ffflush();
      strcpy(title + strlen(title), passagges[j]);
      if (args.toplot) {
        // print args toplot value
        plot(path, args.x, args.y, args.nnodes, title,
             instances[c_inst].node_start);
      }
      printf("tourlenght %lf , time elapsed %ld\n",
             instances[c_inst].tour_length, time(NULL) - tstart);
      // printf("tourlenght %lf\n", get_tour_length(path, args.nnodes,
      // distance_matrix));

      if (j == n_passagges - 1) strcpy(title, "\0");
    }
    sprintf(title + strlen(title), str_startingNode);
    // print tourlenght model time

    OUTPUT_COMMENT("main::main", "Tour lenght: %lf , time elapsed %ld",
                   instances[c_inst].tour_length, time(NULL) - tstart);
    free(path);
  }
  free(distance_matrix);
}
