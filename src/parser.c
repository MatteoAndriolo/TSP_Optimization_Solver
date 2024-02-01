#include "../include/parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/utils.h"

void read_input(Args *args) {
  FILE *fin = fopen(args->input_file, "r");
  if (fin == NULL) {
    FATAL_COMMENT("parser.c:read_input", "File %s not found/not exists",
                  args->input_file);
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
      DEBUG_COMMENT("parser.c:read_input",
                    "Number of nodes in the field DIMENSION: %d", number_nodes);
      args->nnodes = number_nodes;
      args->x = (double *)malloc(number_nodes * sizeof(double));
      args->y = (double *)malloc(number_nodes * sizeof(double));
      DEBUG_COMMENT("parser.c:read_input", "Memory for x and y allocated");
    } else if (strcmp(line, "NODE_COORD_SECTION") == 0) {
      DEBUG_COMMENT("parser.c:read_input",
                    "Reading in the file the NODE_COORD_SECTION and start "
                    "reading the node");
      while (fgets(line, 1024, fin)) {
        if (sscanf(line, "%d %lf %lf", &node_number, &node_x, &node_y) == 3) {
          int nn = node_number - 1;
          args->x[nn] = node_x;
          args->y[nn] = node_y;
          node_saved++;
        } else {
          DEBUG_COMMENT("parser.c:read_input", "Reached end of the file");
          break;
        }
      }
    }
  }

  fclose(fin);

  if (number_nodes != node_saved) {
    FATAL_COMMENT("parser.c:read_input",
                  "field DIMENSION != real number of nodes in the file");
  }

  INFO_COMMENT("parser.c:read_input", "Reading is finished, close the file");
}

void print_arguments(const Args *args) {
  printf(
      "\n\navailable parameters (vers. 16-may-2015) "
      "--------------------------------------------------\n");
  printf("-input %s\n", args->input_file);
  printf("-m %s\n", args->model_type);
  printf("--seed %d\n", args->randomseed);
  printf("--numinstances %d\n", args->num_instances);
  printf("--time %ld\n", args->timelimit);
  if (strcmp(args->grasp, "1") != 0)
    for (int i = 0; i < args->n_probabilities; i++)
      printf("--grasp %lf\n", args->grasp_probabilities[i]);
  printf("--perfprof %s\n", args->perfprof ? "true" : "false");
  if (args->paths_file) printf("--pathin %s\n", args->paths_file);
  printf(
      "------------------------------------------------------------------------"
      "----------------------\n\n");
  ffflush();
}

void parse_model_name(char *model_type, char ***passagges, int *n_passagges) {
  // given a string like "model1.model2.model3" it returns an array of strings
  (*n_passagges) = 0;
  int length = strlen(model_type);
  for (int i = 0; i < length; i++)
    if (model_type[i] == delimiter) {
      (*n_passagges)++;
      model_type[i] = '\0';
    }
  (*n_passagges)++;
  char **p = malloc((*n_passagges) * sizeof(char *));

  int ind = 0;
  for (int i = 0; i < *n_passagges; i++) {
    p[i] = malloc(10 * sizeof(char));
    sscanf(model_type + ind, "%s", p[i]);
    ind += strlen(p[i]) + 1;
  }
  *passagges = (char **)p;
}

void parse_command_line(int argc, char **argv, Args *args) {
  // DEFAULTS
  args->num_instances = 0;
  args->integer_costs = 0;
  args->randomseed = 1;
  args->timelimit = 9999999;
  args->toplot = 0;
  args->n_probabilities = 1;
  args->grasp_probabilities = (double *)malloc(sizeof(double));
  args->grasp_probabilities[0] = 1;
  args->perfprof = false;
  args->paths_file = NULL;
  strcpy(args->input_file, "\0");
  strcpy(args->log_file, "\0");
  strcpy(args->grasp, "1");

  int help = 0;
  if (argc < 1) help = 1;
  for (int i = 1; i < argc; i++) {
    // model type
    if ((strcmp(argv[i], "--model") == 0) | (strcmp(argv[i], "-m") == 0)) {
      strcpy(args->model_type, argv[++i]);
    } else if (strcmp(argv[i], "--int") == 0) {
      args->integer_costs = true;
    } else if (strcmp(argv[i], "--seed") == 0) {
      args->randomseed = abs(atoi(argv[++i]));
    } else if ((strcmp(argv[i], "-file") == 0) |
               (strcmp(argv[i], "-input") == 0) |
               (strcmp(argv[i], "-f") == 0)) {
      strcpy(args->input_file, argv[++i]);
      fflush(stdout);
    } else if ((strcmp(argv[i], "--grasp") == 0) |
               (strcmp(argv[i], "-g") == 0)) {
      args->n_probabilities = 0;
      free(args->grasp_probabilities);
      args->grasp_probabilities = (double *)calloc(10, sizeof(double));
      // PARSE
      strcpy(args->grasp, argv[++i]);
      parse_grasp_probabilities(args->grasp, args->grasp_probabilities,
                                &args->n_probabilities);

      for (int i = 1; i < args->n_probabilities; i++) {
        if (i == 0)
          sprintf(args->str_probabilities, "%.1f_",
                  args->grasp_probabilities[0]);
        else if (i == args->n_probabilities - 1)
          sprintf(args->str_probabilities + strlen(args->str_probabilities),
                  "%.1f", args->grasp_probabilities[i]);
        else
          sprintf(
              args->str_probabilities + strlen(args->str_probabilities),
              "%.1f_",
              args->grasp_probabilities[i] - args->grasp_probabilities[i - 1]);
      }
    } else if ((strcmp(argv[i], "--maxtime") == 0) ||
               (strcmp(argv[i], "--time") == 0) ||
               (strcmp(argv[i], "-t") == 0)) {
      args->timelimit = atol(argv[++i]);
    } else if ((strcmp(argv[i], "-n") == 0) ||
               (strcmp(argv[i], "--numinstances") == 0)) {
      args->num_instances = atoi(argv[++i]);
    } else if ((strcmp(argv[i], "-help") == 0) ||
               (strcmp(argv[i], "--help") == 0)) {
      help = 1;
    } else if (strcmp(argv[i], "--plot") == 0) {
      args->toplot = 1;
      printf("PUTTING PLOTTING AT 1");
    } else if (strcmp(argv[i], "--perfprof") == 0) {
      args->perfprof = true;
    } else if (strcmp(argv[i], "--pathin") == 0) {
      char *pathin = argv[++i];
      args->paths_file = (char *)malloc(strlen(pathin) * sizeof(char));
      args->paths_file = pathin;
    } else if (strcmp(argv[i], "--cperchf") == 0) {
      args->cplex_perchf = atof(argv[++i]);
      if (args->cplex_perchf < 1 || args->cplex_perchf > 100) {
        FATAL_COMMENT("parser.c:parse_command_line",
                      "cplex_perchf must be in [1,100]");
      }
      continue;
    } else if (strcmp(argv[i], "--tabu") == 0) {
      args->tabu_size = atoi(argv[++i]);
      continue;
    }
  }
  if (help) exit(1);
}

void parse_grasp_probabilities(char *grasp, double *probabilities,
                               int *n_probabilities) {
  // clean default values
  *n_probabilities = 0;

  // setup
  char delimiter = '.';
  *n_probabilities = 0;
  int length = strlen(grasp);
  int cur_ind = 0;
  int i;
  DEBUG_COMMENT("parser.c:parser_grasp_probabilities", "grasp=%s", grasp);
  for (i = 0; i < length; i++)
    if (grasp[i] == delimiter) {
      grasp[i] = '\0';
      sscanf(grasp + cur_ind, "%lf", &probabilities[*n_probabilities]);
      DEBUG_COMMENT("parser.c:parser_grasp_probabilities",
                    "probabilities[%d]=%lf vs %s", *n_probabilities,
                    probabilities[*n_probabilities], grasp + cur_ind);
      cur_ind += strlen(grasp + cur_ind) + 1;
      (*n_probabilities)++;
    }
  sscanf(grasp + cur_ind, "%lf", &probabilities[*n_probabilities]);
  DEBUG_COMMENT("parser.c:parser_grasp_probabilities",
                "probabilities[%d]=%lf vs %s", *n_probabilities,
                probabilities[*n_probabilities], grasp + cur_ind);
  cur_ind += strlen(grasp + cur_ind) + 1;
  (*n_probabilities)++;
  DEBUG_COMMENT("parser.c:parser_grasp_probabilities", "n_probabilities=%d",
                *n_probabilities);

  // normalize probabilities
  double sum = 0;
  for (int i = 0; i < *n_probabilities; i++) {
    sum += probabilities[i];
  }
  for (int i = 0; i < *n_probabilities; i++) {
    probabilities[i] /= sum;
  }
  DEBUG_COMMENT("parser.c:parser_grasp_probabilities", "probabilities[%d]=%lf",
                0, probabilities[0]);
  for (int i = 1; i < *n_probabilities; i++) {
    probabilities[i] += probabilities[i - 1];
    DEBUG_COMMENT("parser.c:parser_grasp_probabilities",
                  "probabilities[%d]=%lf", i, probabilities[i]);
  }
}

void readPermutations(const char *filename, int ***array, int *pn, int *ps) {
  FILE *file;
  int i, j;
  int n = -1, s = -1;

  // Open the file
  file = fopen(filename, "r");
  if (file == NULL) {
    perror("Error opening file");
    return;
  }

  // first line contains "s n"
  if (fscanf(file, "%d %d", &n, &s) != 2) {
    fprintf(stderr, "Error reading file at line 1\n");
    fclose(file);
    return;
  }

  // Allocate memory for the array
  *array = (int **)malloc(s * sizeof(int *));
  for (i = 0; i < s; i++) {
    (*array)[i] = (int *)malloc(n * sizeof(int));
  }

  // Read the file and store the values in the array
  for (i = 0; i < s; i++) {
    for (j = 0; j < n; j++) {
      if (fscanf(file, "%d", &(*array)[i][j]) != 1) {
        fprintf(stderr, "Error reading file at line %d\n", i + 1);
        // Free allocated memory in case of error
        for (int k = 0; k <= i; k++) {
          free((*array)[k]);
        }
        free(*array);
        fclose(file);
        return;
      }
    }
  }

  // Close the file
  fclose(file);
  *pn = n;
  *ps = s;
}

void writePermutationsToFile(const char *filename) {
  char *file_dimensions = "data/dimensionMaps";
  FILE *file_dimensions_ptr = fopen(file_dimensions, "r");
  int num_dimensions = -1;
  fscanf(file_dimensions_ptr, "number; %d", &num_dimensions);

  for (int i = 0; i < num_dimensions; i++) {
    int n = -1, s = -1;
    fscanf(file_dimensions_ptr, "%d; %d", &n, &s);
    char *filename = (char *)malloc(100 * sizeof(char));
    sprintf(filename, "data/permutations/%d", n);
    FILE *file = fopen(filename, "w");
    int *array = (int *)malloc(n * sizeof(int));
    if (array == NULL) {
      perror("Memory allocation failed");
      fclose(file);
      return;
    }
    if (file == NULL) {
      perror("Error opening file");
      return;
    }

    // Initialize the array with 1 to n
    for (int i = 0; i < n; i++) {
      array[i] = i;
    }

    fprintf(file, "%d %d\n", n, s);
    // Generate and write s permutations
    for (int i = 0; i < s; i++) {
      shuffle(array, n);
      for (int j = 0; j < n; j++) {
        fprintf(file, "%d ", array[j]);
      }
      fprintf(file, "\n");
    }

    free(array);
    free(filename);
    fclose(file);
  }

  fclose(file_dimensions_ptr);
}
void freePermutations(int n, int s, int ***array) {
  for (int i = 0; i < s; i++) {
    free(array[i]);
  }
  free(array);
}

char *sprintHeaderOutput(Output *output) {
  char *header = (char *)malloc(sizeof(char) * 1000);
  header[0] = '\0';
  char *intermediate = (char *)malloc(sizeof(char) * 1000);
  intermediate[0] = '\0';
  char *buffer = (char *)malloc(sizeof(char) * 1000);
  buffer[0] = '\0';

  for (int i = 0; i < output->n_passagges; i++) {
    sprintf(buffer, "passagge %d; ", i);
    strcat(intermediate, buffer);
  }
  intermediate[strlen(intermediate) - 2] = '\0';

  sprintf(header, "iteration; tlstart; n_passagges; %s; tlfinal; time",
          intermediate);

  free(intermediate);
  free(buffer);

  return header;
}

char *sprintOutput(Output *output) {
  char *str = (char *)malloc(sizeof(char) * 1000);
  str[0] = '\0';
  char *intermediate = (char *)malloc(sizeof(char) * 1000);
  intermediate[0] = '\0';
  char *buffer = (char *)malloc(sizeof(char) * 1000);
  buffer[0] = '\0';

  for (int i = 0; i < output->n_passagges; i++) {
    sprintf(buffer, "%lf; ", output->intermediate_tour_length[i]);
    strcat(intermediate, buffer);
  }
  intermediate[strlen(intermediate) - 2] = '\0';

  sprintf(str, "%d; %lf; %d; %s; %lf; %lf", output->iteration, output->tlstart,
          output->n_passagges, intermediate, output->tlfinal, output->duration);
  printf("%s", str);

  free(intermediate);
  free(buffer);

  return str;
}

void writeOutput(Output **output, FILE *fp, int num_iterations) {
  fprintf(fp, "%s\n", sprintHeaderOutput(output[0]));
  for (int i = 0; i < num_iterations; i++) {
    fprintf(fp, "%s\n", sprintOutput(output[i]));
  }
}
