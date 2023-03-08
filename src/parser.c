#include "parser.h"

void read_input(Instance *inst) // simplified CVRP parser, not all SECTIONs detected
{
    if (inst->verbosity >= 4)
    {
        printf("... START reading the file");
    }

    FILE *fin = fopen(inst->input_file, "r");
    if (fin == NULL)
        print_error(" input file not found!");

    inst->nnodes = -1;
    char line[1024];
    int number_nodes = 0;
    int reading_nodes = 0;
    double node_x, node_y;
    int node_number, node_saved;

    while (fgets(line, 1024, fin))
    {
        // Strip trailing newline
        int len = strlen(line);
        if (line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }

        if (sscanf(line, "DIMENSION : %d", &number_nodes) == 1)
        {
            if (inst->verbosity >= 3)
                printf("Number of nodes in the field DIMENTION: %d \n", number_nodes);
            inst->nnodes = number_nodes;
            inst->x = (double *)calloc(number_nodes, sizeof(double));
            inst->y = (double *)calloc(number_nodes, sizeof(double));
            if (inst->verbosity >= 4)
                printf("Memory for x and y allocated\n");
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
                inst->x[nn] = node_x;
                inst->y[nn] = node_y;
                if (inst->verbosity >= 4)
                {
                    printf("Node %d: (%lf, %lf)\n", node_number, node_x, node_y);
                    printf("Poin %d: (%lf, %lf)\n", nn, inst->x[nn], inst->y[nn]);
                }
                node_saved++;
            }
            else
            {
                // Reached end of node coordinates section
                break;
            }
        }
        else
        {
            if (strcmp(line, "NODE_COORD_SECTION") == 0)
            {
                reading_nodes = 1;
            }
            else if (sscanf(line, "DIMENSION : %d", &number_nodes) == 1)
                ;
        }
    }

    if (inst->verbosity >= 3)
    {
        for (int i = 0; i < number_nodes; i++)
        {
            printf("Point %d: (%lf, %lf)\n", i, inst->x[i], inst->y[i]);
        }

        if (inst->verbosity >= 3)
            printf("Parsed %d nodes.\n", number_nodes);
    }

    if (inst->verbosity >= 2)
        printf("... ENDING reading the file");
    fclose(fin);
}

char doc[] = "A program to parse command-line arguments.";
/* Available options. */
struct argp_option options[] = {
    {"file", 'f', "FILE", 0, "Input file"},
    {"input", 'f', "FILE", 0, "Input file"},
    {"model", 'm', "TYPE", 0, "Model type"},
    {"seed", 's', "SEED", 0, "Random seed"},
    {"threads", 't', "NUM", 0, "Number of threads"},
    {"time_limit", 'l', "LIMIT", 0, "Time limit"},
    {"memory", 'L', "SIZE", 0, "Available memory"},
    {"node_file", 'n', "FILE", 0, "Cplex's node file"},
    {"max_nodes", 'N', "NUM", 0, "Max number of nodes"},
    {"cutoff", 'c', "VALUE", 0, "Master cutoff"},
    {"verbosity", 'v', "NUM", 0, "Set verbosity 1-100"},
    {"int", 'i', 0, 0, "Integer costs"},
    {"help", 'h', 0, 0, "Show help"},
    };

/* Parse a single option. */
error_t parse_option(int key, char *arg, struct argp_state *state)
{
    struct arguments *args = state->input;

    switch (key)
    {
    case 'f':
        strcpy(args->inst.input_file, arg);
        break;
    case 'l':
        args->inst.timelimit = atof(arg);
        break;
    case 'm':
        args->inst.model_type = atoi(arg);
        break;
    case 's':
        args->inst.randomseed = abs(atoi(arg));
        break;
    case 't':
        args->inst.num_threads = atoi(arg);
        break;
    case 'L':
        args->inst.available_memory = atoi(arg);
        break;
    case 'n':
        strcpy(args->inst.node_file, arg);
        break;
    case 'N':
        args->inst.max_nodes = atoi(arg);
        break;
    case 'p':
        strcpy(args->inst.log_file, arg);
        break;
    case 'c':
        args->inst.cutoff = atof(arg);
        break;
    case 'i':
        args->inst.integer_costs = 1;
        break;
    case 'v':
        args->inst.verbosity = atoi(arg);
        break;
    case 'h':
        args->help = 1;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

/* Define the argp parser. */
struct argp argp_parser = {options, parse_option, 0, doc};

/* Parse command-line arguments. */
void parse_command_line(int argc, char **argv, struct arguments *args)
{
    /* Default values. */
    // args->inst.input_file = NULL;
    args->inst.model_type = 0;
    args->inst.randomseed = 0;
    args->inst.num_threads = 0;
    args->inst.timelimit = INFTY;
    args->inst.cutoff = INFTY;
    args->inst.integer_costs = 0;
    args->inst.available_memory = 0;
    args->inst.max_nodes = 0;
    args->inst.verbosity = 1;
    // args->inst.node_file = 0;
    args->help = 0;

    /* Parse arguments. */
    argp_parse(&argp_parser, argc, argv, 0, 0, args);
}

void usage(FILE *fp, int status)
{
    argp_help(&argp_parser, fp, ARGP_HELP_LONG | ARGP_HELP_DOC, "TODO programm name");
    exit(status);
}

void print_arguments(FILE *fp, const Instance *inst)
{
    fprintf(fp, "-c\tcutoff value\t%f\t\n", inst->cutoff);
    fprintf(fp, "-f\tinput file\t%s\t\n", inst->input_file);
    fprintf(fp, "-i\tinteger cost\t%i\t\n", inst->integer_costs);
    fprintf(fp, "-l\ttime limit\t%f\t\n", inst->timelimit);
    fprintf(fp, "-L\tmemory limit\t%i\t\n", inst->available_memory);
    fprintf(fp, "-m\tmodel type\t%i\t\n", inst->model_type);
    fprintf(fp, "-n\tnode file\t%s\t\n", inst->node_file);
    fprintf(fp, "-N\tmax nodes\t%i\t\n", inst->max_nodes);
    fprintf(fp, "-o\toutput log file\t%i\t\n", inst->max_nodes);
    fprintf(fp, "-s\tseed\t\t%i\t\n", inst->randomseed);
    fprintf(fp, "-t\tnum threads\t%i\t\n", inst->num_threads);
    fprintf(fp, "-v\tverbosity\t%i\t\n", inst->verbosity);
}