/* TOFO ggc -o3 or -o4 -o bin/main src/main && bin/main */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../include/greedy.h"
#include "../include/logger.h"
#include "../include/metaheuristic.h"
#include "../include/parser.h"
#include "../include/plot.h"
#include "../include/tabu.h"
#include "../include/tspcplex.h"
#include "../include/utils.h"
#include "../include/vrp.h"

int main(int argc, char **argv) {
  logger_init("example.log");

  // Parsing Arguments ------------------------------------------------------
  Args args;
  parse_command_line(argc, argv, &args);

  // Read TLP library -------------------------------------------------------
  read_input(&args);

  // Parsing model ----------------------------------------------------------
  int n_passagges;
  char **passagges;
  parse_model_name(args.model_type, &passagges, &n_passagges);

  // Manage model selection -------------------------------------------------
  print_arguments(&args);
  srand(args.randomseed);                 // set random seed
  Instance instances[args.num_instances]; // array of instances

  // RUN_MAIN
  for (int c_inst = 0; c_inst < args.num_instances; c_inst++) {
    Instance *inst = &instances[c_inst];
    INSTANCE_initialize(&instances[c_inst], args.timelimit, args.integer_costs,
                        args.nnodes, args.x, args.y, args.n_probabilities,
                        args.grasp_probabilities, args.input_file,
                        args.log_file);

    char title[40] = "\0";

    for (int j = 0; j < n_passagges; j++) {
      INFO_COMMENT("main.c:main", "Generating instance %s", passagges[j]);
      printf("Generating instance %s\n", passagges[j]);
      if (strcmp(passagges[j], "em") == 0) {
        if (inst->grasp->size > 1) {
          ERROR_COMMENT("main.c:main",
                        "Extra mileage not implemented for grasp");
        }
        RUN_MAIN(extra_mileage(&instances[c_inst]));
      } else if (strcmp(passagges[j], "2opt") == 0) {
        RUN_MAIN(two_opt(inst, INFINITY));
      } else if (strcmp(passagges[j], "tabu") == 0) {
        if (j == 0) {
          FATAL_COMMENT("main.c:main",
                        "Tabu search must be used with a starting point");
        }
        RUN_MAIN(two_opt_tabu(inst, INFINITY, initializeTabuList(10, 4)));
      } else if (strcmp(passagges[j], "vns") == 0) {
        RUN_MAIN(vns_k(inst, 4));
      } else if (strcmp(passagges[j], "sa") == 0) {
        RUN_MAIN(simulated_annealling(inst, 1000));
      } else if (strcmp(passagges[j], "gen") == 0) {
        ERROR_COMMENT("main.c:main", "Genetic algorithm not implemented yet");
      } else if (strcmp(passagges[j], "nn") == 0) {
        RUN_MAIN(nearest_neighboor(inst));
      } else if (strcmp(passagges[j], "cplex") == 0) {
        inst->percentageHF = -1;
        inst->solver = SOLVER_BASE;
        TSPopt(inst);
      } else if (strcmp(passagges[j], "cplexbend") == 0) {
        inst->percentageHF = -1;
        inst->solver = SOLVER_BENDER;
        TSPopt(inst);
      } else if (strcmp(passagges[j], "cplexbc") == 0) {
        inst->percentageHF = -1;
        inst->solver = SOLVER_BRANCH_AND_CUT;
        TSPopt(inst);
      } else if (strcmp(passagges[j], "cplexpatch") == 0) {
        inst->percentageHF = -1;
        inst->percentageHF = 80;
        inst->solver = SOLVER_PATCHING_HEURISTIC;
        TSPopt(inst);
      } else if (strcmp(passagges[j], "cplexpost") == 0) {
        inst->percentageHF = -1;
        inst->solver = SOLVER_POSTINGHEU_UCUTFRACT;
        TSPopt(inst);
      } else if (strcmp(passagges[j], "cplexhf") == 0) {
        inst->solver = SOLVER_MH_HARDFIX; // TODO not working
        TSPopt(inst);
      } else if (strcmp(passagges[j], "cplexlb") == 0) {
        inst->solver = SOLVER_MH_LOCBRANCH;

        two_opt(inst, INFINITY);
        TSPopt(inst);
        DEBUG_COMMENT("main.c:main", "back to main");
      } else if (strcmp(passagges[j], "test") == 0) {
        testTabuList();
      } else {
        FATAL_COMMENT("main.c:main", "Model %s not recognized", passagges[j]);
      }

      INSTANCE_saveBestPath(inst);
      ffflush();
      strcpy(title + strlen(title), passagges[j]);

      if (args.toplot) {
        // print args toplot value
        plot(inst->path, args.x, args.y, args.nnodes, title,
             inst->starting_node);
      }
      printf("tourlenght %lf , time elapsed %ld\n", inst->best_tourlength,
             time(NULL) - inst->tstart);
      // printf("tourlenght %lf\n", get_tour_length(inst->path, args.nnodes,
      // inst->distance_matrix));

      if (j == n_passagges - 1)
        strcpy(title, "\0");
      INFO_COMMENT("main.c:main", "Passagge %s completed with tl %lf",
                   passagges[j], inst->best_tourlength);
    }
    // print tourlenght model time
    OUTPUT_COMMENT("main.c:main", "Tour lenght: %lf , time elapsed %ld",
                   inst->best_tourlength, difftime(time(NULL), inst->tstart));
    INFO_COMMENT("main.c:main", "Instance %d completed", c_inst);
    INSTANCE_free(inst);
    INFO_COMMENT("main.c:main", "Instance %d completed", c_inst);
  }
}
