#include "../include/parser.h"

void read_input(Args *args) {
    FILE *fin = fopen(args->input_file, "r");
    if (fin == NULL) {
        FATAL_COMMENT("parser::read_input", "File %s not found/not exists", args->input_file);
    }

    int number_nodes = -1;
    int node_saved = 0;

    char line[1024];
    while (fgets(line, 1024, fin)) {
        int len = strlen(line);
        if (line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        int node_number;
        double node_x, node_y;
        if (sscanf(line, "DIMENSION : %d", &number_nodes) == 1) {
            DEBUG_COMMENT("parser::read_input", "Number of nodes in the field DIMENSION: %d", number_nodes);
            args->nnodes = number_nodes - 1;
            args->x = (double *)malloc(number_nodes * sizeof(double));
            args->y = (double *)malloc(number_nodes * sizeof(double));
            DEBUG_COMMENT("parser::read_input", "Memory for x and y allocated");
        } else if (strcmp(line, "NODE_COORD_SECTION") == 0) {
            DEBUG_COMMENT("parser::read_input", "Reading in the file the NODE_COORD_SECTION and start reading the node");
            while (fgets(line, 1024, fin)) {
                if (sscanf(line, "%d %lf %lf", &node_number, &node_x, &node_y) == 3) {
                    int nn = node_number - 1;
                    args->x[nn] = node_x;
                    args->y[nn] = node_y;
                    node_saved++;
                } else {
                    DEBUG_COMMENT("parser::read_input", "Reached end of the file");
                    break;
                }
            }
        }
    }

    fclose(fin);

    if (number_nodes != node_saved) {
        WARNING_COMMENT("parser::read_input", "field DIMENSION != real number of nodes in the file");
    }

    INFO_COMMENT("parser::read_input", "Reading is finished, close the file");
}


void print_arguments(const Args *args)
{
    printf("\n\navailable parameters (vers. 16-may-2015) --------------------------------------------------\n");
    printf("-input %s\n", args->input_file);
    printf("-m %s\n", args->model_type);
    printf("--seed %d\n", args->randomseed);
    printf("--numinstances %d\n", args->num_instances);
    if (strcmp(args->grasp, "1") != 0)
        for (int i = 0; i < args->n_probabilities; i++)
            printf("--grasp %lf\n", args->grasp_probabilities[i]);
    printf("----------------------------------------------------------------------------------------------\n\n");
}

void parse_model_name(char *model_type, char ***passagges, int *n_passagges)
{
    // given a string like "model1.model2.model3" it returns an array of strings
    (*n_passagges) = 0;
    int length = strlen(model_type);
    for (int i = 0; i < length; i++)
        if (model_type[i] == delimiter)
        {
            (*n_passagges)++;
            model_type[i] = '\0';
        }
    (*n_passagges)++;
    char **p = malloc((*n_passagges) * sizeof(char *));

    int ind = 0;
    for (int i = 0; i < *n_passagges; i++)
    {
        p[i] = malloc(10 * sizeof(char));
        sscanf(model_type + ind, "%s", p[i]);
        ind += strlen(p[i]) + 1;
    }
    *passagges = (char **)p;
}

void parse_command_line(int argc, char **argv, Args *args)
{
    // defaults
    // strcpy(args->model_type, "\0");
    args->num_instances = 1;
    args->integer_costs = 0;
    args->randomseed = 1234;
    args->timelimit = 3600.0;
    args->toplot = 0;
    args->n_probabilities = 0;
    args->grasp_probabilities = malloc(10 * sizeof(double));
    strcpy(args->input_file, "\0");
    strcpy(args->log_file, "\0");
    strcpy(args->grasp, "1");

    int help = 0;
    if (argc < 1)
        help = 1;
    for (int i = 1; i < argc; i++)
    {
        // model type
        if ((strcmp(argv[i], "--model") == 0) | (strcmp(argv[i], "-m") == 0))
        {
            strcpy(args->model_type, argv[++i]);
            continue;
        } // model type
        if (strcmp(argv[i], "--int") == 0)
        {
            args->integer_costs = 1;
            continue;
        } // inteher costs
        if (strcmp(argv[i], "--seed") == 0)
        {
            args->randomseed = abs(atoi(argv[++i]));
            continue;
        } // random seed
        if ((strcmp(argv[i], "-file") == 0) | (strcmp(argv[i], "-input") == 0) | (strcmp(argv[i], "-f") == 0))
        {
            strcpy(args->input_file, argv[++i]);
            fflush(stdout);
            continue;
        } // input file
        if ((strcmp(argv[i], "--grasp") == 0) | (strcmp(argv[i], "-g") == 0))
        {
            strcpy(args->grasp, argv[++i]);
            parse_grasp_probabilities(args->grasp, args->grasp_probabilities, &args->n_probabilities);

            for (int i = 1; i < args->n_probabilities; i++)
            {
                if (i == 0)
                    sprintf(args->str_probabilities, "%.1f_", args->grasp_probabilities[0]);
                else if (i == args->n_probabilities - 1)
                    sprintf(args->str_probabilities + strlen(args->str_probabilities), "%.1f", args->grasp_probabilities[i]);
                else
                    sprintf(args->str_probabilities + strlen(args->str_probabilities), "%.1f_", args->grasp_probabilities[i] - args->grasp_probabilities[i - 1]);
            }
            continue;
        }
        if ((strcmp(argv[i], "--maxtime") == 0) | (strcmp(argv[i], "--time") == 0))
        {
            args->timelimit = atof(argv[++i]);
            continue;
        } // total time limit
        if ((strcmp(argv[i], "-n") == 0) | (strcmp(argv[i], "--numinstances") == 0))
        {
            args->num_instances = atoi(argv[++i]);
        }
        if ((strcmp(argv[i], "-help") == 0) | (strcmp(argv[i], "--help") == 0))
        {
            help = 1;
            continue;
        } // help
        if (strcmp(argv[i], "--plot") == 0 )
        {
            args->toplot = 1;
            printf("PUTTING PLOTTING AT 1");
        }
    }

    if (help)
        exit(1);
}

/*
void parse_grasp_probabilities(char *grasp, double *probabilities, int *n_probabilities )
{
    char delimiter = '.';
    (*n_probabilities) = 0;
    int length = strlen(grasp);
    for (int i = 0; i < length; i++)
        if (grasp[i] == delimiter)
        {
            (*n_probabilities)++;
            grasp[i] = '\0';
        }
    (*n_probabilities)++;

    DEBUG_COMMENT("parser::parser_grasp_probabilities","n_probabilities=%d", *n_probabilities);
    int ind = 0;
    double sum = 0;
    for (int i = 0; i < *n_probabilities; i++)
    {
        char *endptr;
        probabilities[i] = strtod(grasp + ind, &endptr);
        if (endptr == grasp + ind) {
            // conversion failed
            ERROR_COMMENT("parser::parser_grasp_probabilities","conversion failed");
        exit(1);
            return;
        }
        ind = endptr - grasp;
        sum += probabilities[i];
        DEBUG_COMMENT("parser::parser_grasp_probabilities","probabilities[%d]=%lf", i, probabilities[i]);
    }

    for (int i = 0; i < (*n_probabilities); i++)
        probabilities[i] /= sum;
    for (int i = 1; i < (*n_probabilities); i++)
        probabilities[i] = probabilities[i - 1] + probabilities[i];

    DEBUG_COMMENT("parser::parser_grasp_probabilities","probabilities=%lf,%lf,%lf", probabilities[0], probabilities[1], probabilities[2]);
}*/

void parse_grasp_probabilities(char *grasp, double *probabilities, int *n_probabilities)
{
    DEBUG_COMMENT("parser::parser_grasp_probabilities", "grasp=%s", grasp);
    char delimiter = '.';
    *n_probabilities = 0;
    int length = strlen(grasp);
    int cur_ind=0;
    int i;
    for( i = 0; i < length; i++)
        if (grasp[i] == delimiter)
        {
            grasp[i] = '\0';
            sscanf(grasp + cur_ind, "%lf", &probabilities[*n_probabilities]);
            DEBUG_COMMENT("parser::parser_grasp_probabilities", "probabilities[%d]=%lf vs %s", *n_probabilities, probabilities[*n_probabilities], grasp + cur_ind);
            cur_ind += strlen(grasp + cur_ind) + 1;
            (*n_probabilities)++;
        }
    sscanf(grasp + cur_ind, "%lf", &probabilities[*n_probabilities]);
    DEBUG_COMMENT("parser::parser_grasp_probabilities", "probabilities[%d]=%lf vs %s", *n_probabilities, probabilities[*n_probabilities], grasp + cur_ind);
    cur_ind += strlen(grasp + cur_ind) + 1;
    (*n_probabilities)++;
    DEBUG_COMMENT("parser::parser_grasp_probabilities", "n_probabilities=%d", *n_probabilities);

    // normalize probabilities
    double sum = 0;
    for (int i = 0; i < *n_probabilities; i++)
    {
        sum += probabilities[i];
    }
    for (int i = 0; i < *n_probabilities; i++)
    {
        probabilities[i] /= sum;
    }
    DEBUG_COMMENT("parser::parser_grasp_probabilities", "probabilities[%d]=%lf", 0, probabilities[0]);
    for (int i = 1; i < *n_probabilities; i++)
    {
        probabilities[i] += probabilities[i - 1];
        DEBUG_COMMENT("parser::parser_grasp_probabilities", "probabilities[%d]=%lf", i, probabilities[i]);
    }

}

/*
void parse_grasp_probabilities(char *grasp, double *probabilities, int *n_probabilities)
{
    char delimiter = '.';
    (*n_probabilities) = 0;
    int length = strlen(grasp);
    for (int i = 0; i < length; i++)
        if (grasp[i] == delimiter)
        {
            (*n_probabilities)++;
            grasp[i] = '\0';
        }
    (*n_probabilities)++;

    DEBUG_COMMENT("parser::parser_grasp_probabilities","n_probabilities=%d", *n_probabilities);
    int ind = 0;
    int sum = 0;
    for (int i = 0; i < *n_probabilities; i++)
    {
        sscanf(grasp + ind, "%lf", probabilities+i);
        ind += strlen(grasp + ind) + 1;
        // p[i]=strtod(t, t+strlen(t));
        sum += probabilities[i];
    }

    for (int i = 0; i < (*n_probabilities); i++)
        probabilities[i] /= sum;
    for (int i = 1; i < (*n_probabilities); i++)
        probabilities[i] = probabilities[i - 1] + probabilities[i];
}
*/
