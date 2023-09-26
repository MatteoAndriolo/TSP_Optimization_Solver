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

    // Parsing model ----------------------------------------------------------
    int n_passagges;
    char **passagges;
    parse_model_name(args.model_type, &passagges, &n_passagges);

    // Manage model selection -------------------------------------------------
    print_arguments(&args);
    srand(args.randomseed);                  // set random seed
    Instance instances[args.num_instances];  // array of instances

    // RUN
    for (int c_inst = 0; c_inst < args.num_instances; c_inst++) {
        Instance *inst = &instances[c_inst];
        instance_initialize(&instances[c_inst], args.timelimit,
                args.integer_costs, args.nnodes, args.x, args.y,
                args.n_probabilities, args.grasp_probabilities,
                args.input_file, args.log_file);

        char title[40] = "\0";

        for (int j = 0; j < n_passagges; j++) {
            INFO_COMMENT("main::main", "Generating instance %s", passagges[j]);
            printf("Generating instance %s\n", passagges[j]);
            if (strcmp(passagges[j], "em") == 0) {
                if(extra_mileage(&instances[c_inst])==FAILURE)
                {
                    ERROR_COMMENT("main::main", "Extra mileage failed");
                }
            } else if (strcmp(passagges[j], "2opt") == 0) {
                two_opt(inst->distance_matrix, inst->nnodes, inst->path,
                        &(inst->tour_length), INFINITY,
                        inst->max_time);
            } else if (strcmp(passagges[j], "tabu") == 0) {
                if (j == 0) {
                    FATAL_COMMENT("main::main",
                            "Tabu search must be used with a starting point");
                }
                tabu_search(inst->distance_matrix, inst->path, instances[c_inst].nnodes,
                        &(inst->tour_length), (int)(args.nnodes / 20),
                        (int)(args.nnodes / 8), 2);
            } else if (strcmp(passagges[j], "vns") == 0) {
                ERROR_COMMENT("main::main", "VNS not implemented yet");
            } else if (strcmp(passagges[j], "sa") == 0) {
                simulate_anealling(inst->distance_matrix, inst->path, instances[c_inst].nnodes,
                        &inst->tour_length, 10000, 100);
            } else if (strcmp(passagges[j], "gen") == 0) {
                ERROR_COMMENT("main::main", "Genetic algorithm not implemented yet");
            } else if (strcmp(passagges[j], "nn") == 0) {
                nearest_neighboor(inst);
            // } else if (strcmp(passagges[j], "nng") == 0) {
            //     nearest_neighboor_grasp(inst);
            } else if (strcmp(passagges[j], "vpn") == 0) {
                vnp_k(inst->distance_matrix, inst->path, instances[c_inst].nnodes,
                        &inst->tour_length, 5, 4);
            } else if (strcmp(passagges[j], "test") == 0) {
                test_buffer();
            } else {
                FATAL_COMMENT("main::main", "Model %s not recognized", passagges[j]);
            }
            ffflush();
            strcpy(title + strlen(title), passagges[j]);

            if (args.toplot) {
                // print args toplot value
                plot(inst->path, args.x, args.y, args.nnodes, title,
                        inst->starting_node);
            }
            printf("tourlenght %lf , time elapsed %ld\n",
                    inst->tour_length, time(NULL) - inst->tstart);
            // printf("tourlenght %lf\n", get_tour_length(inst->path, args.nnodes,
            // inst->distance_matrix));

            if (j == n_passagges - 1) strcpy(title, "\0");
        }
        // print tourlenght model time

        OUTPUT_COMMENT("main::main", "Tour lenght: %lf , time elapsed %ld",
                inst->tour_length, time(NULL) - inst->tstart);
        INFO_COMMENT("main::main", "Instance %d completed", c_inst);
        instance_destroy(inst);
        INFO_COMMENT("main::main", "Instance %d completed", c_inst);
    }
}
