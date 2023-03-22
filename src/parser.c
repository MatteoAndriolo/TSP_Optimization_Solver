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

void parse_model_name(char *model_type, char ***passagges, int *n_passagges)
{
    (*n_passagges)=0;
    int length = strlen(model_type);
    for (int i = 0; i < length; i++)
        if (model_type[i] == '.')
        {
            (*n_passagges)++;
            model_type[i] = '\0';
        }
    (*n_passagges)++;
    char** p=malloc((*n_passagges)*sizeof(char*));

    int ind = 0;
    for (int i = 0; i < *n_passagges ; i++)
    {
        p[i] = malloc(10 * sizeof(char));
        sscanf(model_type + ind, "%s", p[i]);
        ind += strlen(p[i]) + 1;
    }
	for(int i=0;i<*n_passagges;i++){
		printf("%s\n",p[i]);
	}
    *passagges=(char**)p;
}

void print_arguments(const Args *args)
{
    printf("\n\navailable parameters (vers. 16-may-2015) --------------------------------------------------\n");
    printf("-input %s\n", args->input_file);
    printf("-m %s\n", args->model_type);
    printf("--seed %d\n", args->randomseed);
    printf("--num_argsances %d\n", args->num_instances);
    printf("----------------------------------------------------------------------------------------------\n\n");
}

void parse_command_line(int argc, char **argv, Args *args)
{
    // defaults
    // strcpy(args->model_type, "\0");
    args->integer_costs = 0;
    args->randomseed = 1234;
    args->timelimit = 3600.0;
    strcpy(args->input_file, "\0");
    strcpy(args->log_file, "\0");

    args->num_instances = 0;
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
          // input file
        if ((strcmp(argv[i], "-file") == 0) | (strcmp(argv[i], "-input") == 0) | (strcmp(argv[i], "-f") == 0))
        {
            strcpy(args->input_file, argv[++i]);
            printf("input file: %s\n", args->input_file);
            fflush(stdout);
            continue;
        } // input file
          // time limit
        if ((strcmp(argv[i], "--maxtime") == 0) | (strcmp(argv[i], "--time") == 0))
        {
            args->timelimit = atof(argv[++i]);
            continue;
        } // total time limit
        if ((strcmp(argv[i], "-n") == 0) | (strcmp(argv[i], "--numinstnces") == 0))
        {
            args->num_instances = atoi(argv[++i]);
        }
        // if ( strcmp(argv[i],"-memory") == 0 ) { inst->available_memory = atoi(argv[++i]); continue; }	// available memory (in MB)
        // if ( strcmp(argv[i],"-node_file") == 0 ) { strcpy(inst->node_file,argv[++i]); conggtinue; }		// cplex's node file
        // if ( strcmp(argv[i],"-max_nodes") == 0 ) { inst->max_nodes = atoi(argv[++i]); continue; } 		// max n. of nodes
        // if ( strcmp(argv[i],"-cutoff") == 0 ) { inst->cutoff = atof(argv[++i]); continue; }				// master cutoff
        if ((strcmp(argv[i], "-help") == 0) | (strcmp(argv[i], "--help") == 0))
        {
            help = 1;
            continue;
        } // help
    }

    print_arguments(args);

    if (help)
        exit(1);
}
