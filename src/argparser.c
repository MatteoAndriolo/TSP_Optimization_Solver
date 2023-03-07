#include<stdio.h>
#include"vrp.h"
#include <argp.h>


static char doc[] = "A program to parse command-line arguments.";
/* Available options. */
static struct argp_option options[] = {
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
    {0}
};

/* Struct for storing command-line arguments. */
struct arguments {
    Instance inst;
    int help;
};

/* Parse a single option. */
static error_t parse_option(int key, char *arg, struct argp_state *state)
{
    struct arguments *args = state->input;

    switch (key) {
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
static struct argp argp_parser = {options, parse_option, 0, doc};

/* Parse command-line arguments. */
void parse_command_line(int argc, char  **argv, struct arguments *args)
{
    /* Default values. */
    //args->inst.input_file = NULL;
    args->inst.model_type = 0;
    args->inst.randomseed = 0;
    args->inst.num_threads = 0;
    args->inst.timelimit = INFTY;
    args->inst.cutoff = INFTY;
    args->inst.integer_costs = 0;
    args->inst.available_memory = 0;
    args->inst.max_nodes = 0;
    //args->inst.node_file = 0;
    args->help = 0;

    /* Parse arguments. */
    argp_parse(&argp_parser, argc, argv, 0, 0, args);
}

static void usage(FILE *fp, int status) {
    argp_help(&argp_parser, fp, ARGP_HELP_LONG | ARGP_HELP_DOC, "TODO programm name");
    exit(status);
}

static void print_arguments(FILE *fp, struct arguments* args){
    //TODO complete
    fprintf(fp,"%s", args->inst.input_file);
    fprintf(fp,"%f", args->inst.timelimit );
    fprintf(fp,"%i", args->inst.model_type );
    fprintf(fp,"%i", args->inst.randomseed );
    fprintf(fp,"%i", args->inst.num_threads );
    fprintf(fp,"%i", args->inst.available_memory );
    fprintf(fp,"%s", args->inst.node_file);
    fprintf(fp,"%i", args->inst.max_nodes );
    fprintf(fp,"%f", args->inst.cutoff );
    fprintf(fp,"%i", args->inst.integer_costs );
    fprintf(fp,"%i", args->inst.verbosity);
    fprintf(fp,"%i", args->help);
}

int main(int argc, char  *argv[])
{
    struct arguments args;
    parse_command_line(argc, argv, &args);

    if(args.help){
        usage(stdout, 0);
    }
    if(args.inst.verbosity>10){ 
        argp_help(&argp_parser, stdout, ARGP_HELP_LONG | ARGP_HELP_DOC, "TODO programm name");
        printf(args.inst.input_file);
    }


    return 0;
}
