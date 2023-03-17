#include "parser.h"
#include "logger.h"

void read_input(Instance *inst) // simplified CVRP parser, not all SECTIONs detected
{

    DEBUG_COMMENT("parser::read_input","Opening the TSP file in reading mode");
    
    FILE *fin = fopen(inst->input_file, "r");

    if (fin == NULL)
        ERROR_COMMENT("parser::read_input","File not found/not exists");
    
    DEBUG_COMMENT("parser::read_input","inizialize the variables");
    inst->nnodes = -1;
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

        if (sscanf(line, "DIMENSION : %d", &number_nodes) == 1)//add optional blank or non blank 
        {
            
            DEBUG_COMMENT("parser::read_input","Number of nodes in the field DIMENSION: %d", number_nodes);

            inst->nnodes = number_nodes-1;
            inst->x = (double *)calloc(number_nodes, sizeof(double));//TODO ? MALLOC IS BETTER 
            inst->y = (double *)calloc(number_nodes, sizeof(double));

            DEBUG_COMMENT("parser::read_input","Memory for x and y allocated");
        }

        if (reading_nodes)
        {
            /*
            From each line take the node_num, allocate after a fixed memory
            with calloc of size of nom_nodes, store inside pair the 2 coordinates
            */

            if (sscanf(line, "%d %lf %lf", &node_number, &node_x, &node_y) == 3)//TODO: can use %*d to just read and don't use it 
            {
                int nn = node_number - 1;
                inst->x[nn] = node_x;
                inst->y[nn] = node_y;
                DEBUG_COMMENT("parser::read_input","[NodesIndex: ( x , y ) ] [%d: (%lf, %lf)]", nn, inst->x[nn], inst->y[nn]);
                node_saved++;
            }
            else
            {
                DEBUG_COMMENT("parser::read_input","Reached end of the file");
                break;
            }
        }
        else
        {
            if (strcmp(line, "NODE_COORD_SECTION") == 0)
            {
                DEBUG_COMMENT("parser::read_input","Reading in the file the NODE_COORD_SECTION and start reading the node");
                reading_nodes = 1;
            }
            else if (sscanf(line, "DIMENSION : %d", &number_nodes) == 1){
                ERROR_COMMENT("parser::read_input","The file does not contain any node");
            }
        }
    }

    if(number_nodes != node_saved){
        WARNING_COMMENT("parser::read_input","field DIMENTION != real number of nodes in the file");
    }

    INFO_COMMENT("parser::read_input","Reading is finished, close the file");

    fclose(fin);
}

char doc[] = "A program to parse command-line arguments.";
/* Available options. */
struct argp_option options[] = {
    {"file", 'f', "FILE", 0, "Input file"},
    {"input", 'f', "FILE", 0, "Input file"},
    {"model", 'm', "TYPE", 0, "Model type"},
    {"model", 'M', 0, 0, "Show list models"},
    {"seed", 's', "SEED", 0, "Random seed"},
    {"threads", 't', "NUM", 0, "Number of threads"},
    {"time_limit", 'l', "LIMIT", 0, "Time limit"},
    {"memory", 'L', "SIZE", 0, "Available memory"},
    {"node_file", 'n', "FILE", 0, "Cplex's node file"},
    {"max_nodes", 'N', "NUM", 0, "Max number of nodes"},
    {"cutoff", 'c', "VALUE", 0, "Master cutoff"},
    //{"verbosity", 'v', "NUM", 0, "Set verbosity 1-100"},
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
    // case 'g':
    //     char *grasp_prob;
    //     strcpy(grasp_prob, arg);
    //     char *token;
    //     int integers[5];
    //     int i = 0;

    //     token = strtok(grasp_prob, ",");
    //     while (token != NULL)
    //     {
    //         integers[i++] = atoi(token);
    //         token = strtok(NULL, ",");
    //     }

    //     printf("Array of integers: ");
    //     for (i = 0; i < 4; i++)
    //     {
    //         if (i < sizeof(integers) / sizeof(integers[0]))
    //         {
    //             printf("%d ", integers[i]);
    //         }
    //         else
    //         {
    //             printf("0 ");
    //         }
    //     }
    //     printf("\n");

    case 'l':
        args->inst.timelimit = atof(arg);
        break;
    case 'm':
        args->inst.model_type = atoi(arg);
        break;
    case 'M':
        args->help_models= 1;
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
    // case 'v':
    //     args->inst.verbosity = atoi(arg);
    //     break;
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
    args->inst.randomseed = 23;
    args->inst.num_threads = 0;
    args->inst.timelimit = INFTY;
    args->inst.cutoff = INFTY;
    args->inst.integer_costs = 0;
    args->inst.available_memory = 0;
    args->inst.max_nodes = 0;
    args->inst.zbest = INFINITY;
    //args->inst.verbosity = 1;
    // args->inst.node_file = 0;

    args->help = 0;
    args->help_models = 0;

    /* Parse arguments. */
    argp_parse(&argp_parser, argc, argv, 0, 0, args);
}

void usage(FILE *fp, int status)
{
    argp_help(&argp_parser, fp, ARGP_HELP_LONG | ARGP_HELP_DOC, "TODO programm name");
    exit(status);
}
void show_models(FILE *fp, int status)
{
    fprintf(fp, "Available models:");
    fprintf(fp, "--------------- Greedy Heuristics ---------------\n");
    fprintf(fp, "1)\tNearest Neighboor\n");
    fprintf(fp, "2)\tExtra Mileage\n");
    fprintf(fp, "1)\tModified Extra Mileage\n");
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
    //fprintf(fp, "-v\tverbosity\t%i\t\n", inst->verbosity);
}