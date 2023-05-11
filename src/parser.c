#include "parser.h"

void read_input(Args *args) // simplified CVRP parser, not all SECTIONs detected
{

    DEBUG_COMMENT("parser::read_input", "Opening the TSP file in reading mode");

    FILE *fin = fopen(args->input_file, "r");

    if (fin == NULL)
    {
        ERROR_COMMENT("parser::read_input", "File not found/not exists");
        exit(1);
    }

    DEBUG_COMMENT("parser::read_input", "inizialize the variables");
    args->nnodes = -1;
    char line[1024];
    int number_nodes = 0;
    int reading_nodes = 0;
    double node_x, node_y;
    int node_number;
    int node_saved = 0;

    while (fgets(line, 1024, fin))
    {
        // Strip trailing newline
        int len = strlen(line);
        if (line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }

        if (sscanf(line, "DIMENSION : %d", &number_nodes) == 1) // add optional blank or non blank
        {

            DEBUG_COMMENT("parser::read_input", "Number of nodes in the field DIMENSION: %d", number_nodes);
            args->nnodes = number_nodes - 1;
            args->x = (double *)malloc(number_nodes * sizeof(double));
            args->y = (double *)malloc(number_nodes * sizeof(double));

            DEBUG_COMMENT("parser::read_input", "Memory for x and y allocated");
        }

        if (reading_nodes)
        {
            /*
            From each line take the node_num, allocate after a fixed memory
            with calloc of size of nom_nodes, store inside pair the 2 coordinates
            */

            if (sscanf(line, "%d %lf %lf", &node_number, &node_x, &node_y) == 3)
            {
                int nn = node_number - 1;
                args->x[nn] = node_x;
                args->y[nn] = node_y;
                DEBUG_COMMENT("parser::read_input", "[NodesIndex: ( x , y ) ] [%d: (%lf, %lf)]", nn, args->x[nn], args->y[nn]);
                node_saved++;
            }
            else
            {
                DEBUG_COMMENT("parser::read_input", "Reached end of the file");
                break;
            }
        }
        else
        {
            if (strcmp(line, "NODE_COORD_SECTION") == 0)
            {
                DEBUG_COMMENT("parser::read_input", "Reading in the file the NODE_COORD_SECTION and start reading the node");
                reading_nodes = 1;
            }
            else if (sscanf(line, "DIMENSION : %d", &number_nodes) == 1)
            {
                ERROR_COMMENT("parser::read_input", "The file does not contain any node");
            }
        }
    }

    if (number_nodes != node_saved)
    {
        WARNING_COMMENT("parser::read_input", "field DIMENTION != real number of nodes in the file");
    }

    INFO_COMMENT("parser::read_input", "Reading is finished, close the file");

    fclose(fin);
}

void print_arguments(const Args *args)
{
    printf("\n\navailable parameters (vers. 16-may-2015) --------------------------------------------------\n");
    printf("-input %s\n", args->input_file);
    printf("-m %s\n", args->model_type);
    printf("--seed %d\n", args->randomseed);
    printf("--num_instances %d\n", args->num_instances);
    if (strcmp(args->grasp, "1") != 0)
        for (int i = 0; i < args->n_probabilities; i++)
            printf("--grasp %lf\n", args->grasp_probabilities[i]);
    printf("----------------------------------------------------------------------------------------------\n\n");
}

void parse_model_name(char *model_type, char ***passagges, int *n_passagges)
{
    (*n_passagges) = 0;
     char delimiter = '.';
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
        if ((strcmp(argv[i], "-n") == 0) | (strcmp(argv[i], "--numinstnces") == 0))
        {
            args->num_instances = atoi(argv[++i]);
        }
        if ((strcmp(argv[i], "-help") == 0) | (strcmp(argv[i], "--help") == 0))
        {
            help = 1;
            continue;
        } // help
        if ((strcmp(argv[i], "--plot") == 0) == 0)
        {
            args->toplot = 1;
        }

        // if ( strcmp(argv[i],"-memory") == 0 ) { inst->available_memory = atoi(argv[++i]); continue; }	// available memory (in MB)
        // if ( strcmp(argv[i],"-node_file") == 0 ) { strcpy(inst->node_file,argv[++i]); conggtinue; }		// cplex's node file
        // if ( strcmp(argv[i],"-max_nodes") == 0 ) { inst->max_nodes = atoi(argv[++i]); continue; } 		// max n. of nodes
        // if ( strcmp(argv[i],"-cutoff") == 0 ) { inst->cutoff = atof(argv[++i]); continue; }				// master cutoff
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
    int cur_ind = 0;
    int i;
    for (i = 0; i < length; i++)
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
void write_csv(int N_ALGORITHMS, int N_INSTANCES, double *times, int j)
{
    printf("j: %d\n", j);
    if (j == 0){
        // in first run write also the header
        FILE *fp = fp = fopen("./performance/performance_data.csv", "w");
        if (fp == NULL)
            printf("Error: Failed to open file\n");
        fprintf(fp, "%d", N_ALGORITHMS);
        for (int i = 1; i <= N_ALGORITHMS; i++)
        {
            fprintf(fp, ", algorithm %d", i);
        }
        fprintf(fp, "\n");
        fclose(fp);
    }

    FILE *fp;
    fp = fopen("./performance/performance_data.csv", "a");
    if (fp == NULL)
        printf("Error: Failed to open file\n");

    fprintf(fp, "instance: %d", j);
    for (int i = 0; i < N_ALGORITHMS; i++)
    {
        double time = times[j * N_ALGORITHMS + i];
        fprintf(fp, ", %f", time);
        printf("instance %d written, times: %f, number in times : %d\n", j, time, j * N_ALGORITHMS + i);
    }
    
    fprintf(fp, "\n");

    // Close the CSV file
    fclose(fp);
}