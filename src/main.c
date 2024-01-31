/* TOFO ggc -o3 or -o4 -o bin/main src/main && bin/main */
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../include/greedy.h"
#include "../include/logger.h"
#include "../include/metaheuristic.h"
#include "../include/parser.h"
#include "../include/plot.h"
#include "../include/refinement.h"
#include "../include/tspcplex.h"
#include "../include/utils.h"
#include "../include/vrp.h"

int main(int argc, char **argv) {
  logger_init("example.log");

  // Parsing Arguments ------------------------------------------------------
  Args args;
  parse_command_line(argc, argv, &args);
  srand(args.randomseed);  // set random seed

  // Parsing model ----------------------------------------------------------
  int n_passagges;
  char **passagges;
  parse_model_name(args.model_type, &passagges, &n_passagges);

  // Read TLP library -------------------------------------------------------
  read_input(&args);

  print_arguments(&args);

  // Manage model selection -------------------------------------------------
  int **array;
  int n = -1, s = -1;
  if (args.perfprof) {
    if (access(args.paths_file, F_OK) == -1) {
      FATAL_COMMENT("main.c:main", "File %s not found", args.paths_file);
    }

    readPermutations(args.paths_file, &array, &n, &s);

    if (n != args.nnodes) {
      FATAL_COMMENT("main.c:main",
                    "Number of nodes in %s is different from the "
                    "number of nodes in the instance %d",
                    args.paths_file, n, args.nnodes);
    }
    args.num_instances = s;
  }

  Instance instances[args.num_instances];  // array of instances
  Output **output = (Output **)malloc(args.num_instances * sizeof(Output *));
  // RUN_MAIN
  for (int c_inst = 0; c_inst < args.num_instances; c_inst++) {
    Instance *inst = &instances[c_inst];
    INSTANCE_initialize(&instances[c_inst], args.timelimit, args.integer_costs,
                        args.nnodes, args.x, args.y, args.n_probabilities,
                        args.grasp_probabilities, args.input_file,
                        args.log_file, args.perfprof);
    // INSTANCE_setPath(inst, array[c_inst]);
    if (args.perfprof) {
      FREE(inst->path);
      inst->path = array[c_inst];
      inst->tour_length = INSTANCE_calculateTourLength(inst);
      inst->best_tourlength = inst->tour_length;
    }
    printf("PATH SETTED %d\n", c_inst);
    double *intermediate_tour_length =
        (double *)malloc(sizeof(double) * n_passagges);

    char title[40] = "\0";

    inst->tstart = clock();

    Output *out = (Output *)malloc(sizeof(Output));
    *out = (Output){
        c_inst, inst->tour_length, n_passagges, intermediate_tour_length, -1,
        -1,
    };

    for (int j = 0; j < n_passagges; j++) {
      intermediate_tour_length[j] = inst->best_tourlength;

      INFO_COMMENT("main.c:main", "Generating instance %s", passagges[j]);

      printf("Passagge %s %lf\n", passagges[j], inst->best_tourlength);

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
        int maxJump = (int)(inst->nnodes / 3);
        int minJump = 4;
        int iterations = inst->nnodes * 2;
        RUN_MAIN(vns_k(inst, maxJump, minJump, iterations));
      } else if (strcmp(passagges[j], "sa") == 0) {
        RUN_MAIN(simulated_annealling(inst, 1000));
      } else if (strcmp(passagges[j], "gen") == 0) {
        inst->population_size = 100;
        inst->ngenerations = 300;
        RUN_MAIN(genetic_algorithm(inst));
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
        inst->percentageHF = 70;
        inst->solver = SOLVER_MH_HARDFIX;
        TSPopt(inst);
      } else if (strcmp(passagges[j], "cplexlb") == 0) {
        inst->solver = SOLVER_MH_LOCBRANCH;
        inst->percentageLB = 0.1;
        TSPopt(inst);
        DEBUG_COMMENT("main.c:main", "back to main");
      } else if (strcmp(passagges[j], "test") == 0) {
        testTabuList();
      } else if (strcmp(passagges[j], "generate") == 0) {
        writePermutationsToFile("data/dimensions.txt");
        freePermutations(n, s, &array);
      } else {
        FATAL_COMMENT("main.c:main", "Model %s not recognized", passagges[j]);
      }

      INSTANCE_saveBestPath(inst);
      memcpy(inst->path, inst->best_path, inst->nnodes);
      inst->tour_length = inst->best_tourlength;
      strcat(title, passagges[j]);
      if (j == n_passagges - 1) strcat(title, "\0");

      if (args.toplot) {
        // print args toplot value
        plot(inst->path, args.x, args.y, args.nnodes, title,
             inst->starting_node);
      }

      INFO_COMMENT("main.c:main", "Passagge %s completed with tl %lf",
                   passagges[j], inst->best_tourlength);
      printf("Passagge %s %lf\n", passagges[j], inst->best_tourlength);
    }  // END passagges

    INFO_COMMENT("main.c:main", "exited passagges");

    double duration = (double)(clock() - inst->tstart) / CLOCKS_PER_SEC;
    printf("tourlenght %lf , time elapsed %lf\n", inst->best_tourlength,
           duration);
    out->tlfinal = inst->best_tourlength;
    out->duration = duration;
    output[c_inst] = out;
    // print tourlenght model time

    // OUTPUT_COMMENT("main.c:main", "Tour lenght: %lf , time elapsed %f",
    //                inst->best_tourlength, duration);

    INSTANCE_free(inst);
    if (out) {
      FREE(out);
    }
    if (intermediate_tour_length) {
      FREE(intermediate_tour_length);
    }
  }

  if (args.perfprof) {
    char *filename = (char *)malloc(sizeof(char) * 1000);
    sprintf(filename, "data/out/%s_%s.csv", args.model_type,
            basename(args.input_file));
    FILE *fpout = fopen(filename, "w");
    if (fpout == NULL) {
      FATAL_COMMENT("main.c:main", "Error opening file output.csv");
    }
    writeOutput(output, fpout, args.num_instances);
    free(filename);
  }
  if (output) {
    FREE(output);
  }
  INFO_COMMENT("main.c:main", "EXIT");
}
