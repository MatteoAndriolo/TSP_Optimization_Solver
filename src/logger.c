#include "logger.h"

static FILE *log_file;

void logger_init(const char *log_filename)
{
    log_file = fopen(log_filename, "w");
    if (log_file == NULL)
    {
        ERROR_COMMENT("logger::logger_init", "File not found/not exists");
    }
}

void ffflush()
{
    fflush(log_file);
}

void log_path(const int *path, int nnodes)
{
    char c[20000];
    sprintf(c, "[");
    for (int i = 0; i < nnodes; i++)
    {
        sprintf(c + strlen(c), "%d,", path[i]);
    }
    sprintf(c + strlen(c), "]");
    DEBUG_COMMENT("logger::log_path", "%s", c);
}

char *getPath(const int *path, int nnodes)
{
    char *path_str = (char *)malloc(sizeof(char) * nnodes * 5);
    sprintf(path_str, "[");
    for (int i = 0; i < nnodes; i++)
    {
        if (i % 100 == 0)
            sprintf(path_str + strlen(path_str), "\n%d\t", i);
        sprintf(path_str + strlen(path_str), "\t%d,", path[i]);
    }
    sprintf(path_str + strlen(path_str), "]");
    return path_str;
}

char *getPathDBL(const double *path, int nnodes)
{
    int initialSize = nnodes * 10;
    int currentSize = initialSize;

    char *path_str = (char *)malloc(sizeof(char) * initialSize);
    if (path_str == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }

    strcpy(path_str, "[");
    int remainingSize = currentSize - strlen(path_str);

    for (int i = 0; i < nnodes; i++)
    {
        int requiredSize = snprintf(NULL, 0, " %1.3lf, ", path[i]);
        if (requiredSize > remainingSize)
        {
            currentSize += requiredSize;
            path_str = (char *)realloc(path_str, sizeof(char) * currentSize);
            if (path_str == NULL)
            {
                fprintf(stderr, "Memory reallocation failed.\n");
                return NULL;
            }
            remainingSize = currentSize - strlen(path_str);
        }
        sprintf(path_str + strlen(path_str), " %1.3lf,", path[i]);
        remainingSize = currentSize - strlen(path_str);
    }

    sprintf(path_str + strlen(path_str) - 1, "]");
    return path_str;
}

// char *getPathDBL(const double *path, int nnodes)
// {
//     char *path_str = (char *)malloc(sizeof(char) * nnodes * 10);
//     sprintf(path_str, "[");
//     for (int i = 0; i < nnodes; i++)
//     {
//         sprintf(path_str + strlen(path_str), "\t%lf,", path[i]);
//     }
//     sprintf(path_str + strlen(path_str), "]");
//     return path_str;
// }

void log_distancematrix(const double *distance_matrix, int nnodes)
{
    fprintf(log_file, "%lf;", distance_matrix[0]);
    for (int i = 1; i < pow(nnodes, 2); i++)
    {
        if (i % nnodes == 0)
            fprintf(log_file, "\n");
        fprintf(log_file, "%lf;", distance_matrix[i]);
    }
    fprintf(log_file, "\n");
}

void log_message(LogLevel level, const char *namefile_and_func, const char *format, ...)
{
    // Get the current time
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char combined_str[200];

    // Get the log level string
    const char *level_str;
    switch (level)
    {
    case DEBUG:
        level_str = "DEBUG";
        break;
    case INFO:
        level_str = "INFO";
        break;
    case WARNING:
        level_str = "WARNING";
        break;
    case ERROR:
        level_str = "ERROR";
        break;
    case CRITICAL:
        level_str = "CRITICAL";
        break;
    case OUTPUT:
        level_str = "OUTPUT";
        break;
    default:
        level_str = "";
        break;
    }

    // Write the log message to the log file
    va_list args;
    sprintf(combined_str, "%s | %s", namefile_and_func, format);
    va_start(args, format);
    fprintf(log_file, "%04d-%02d-%02d %02d:%02d:%02d [%s] ",
            tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
            tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, level_str);

    vfprintf(log_file, combined_str, args);
    fprintf(log_file, "\n");
    va_end(args);
    ffflush();
}

void log_output_inst(const Instance *inst)
{
    fprintf(log_file, "$STAT\tmod\tstart\tzbest\ttime_limit\tseed\tnnodes\tinput_file\n");
    fprintf(log_file, "$STAT\t%d\t%d\t%lf\t%lf\t%d\t%d\t%s\n",
            inst->model_type,
            inst->node_start,
            inst->zbest,     // double
            inst->timelimit, // double
            inst->randomseed,
            inst->nnodes,
            inst->input_file);
}

void log_output(int model_type, int node_start, double zbest, double timelimit, int randomseed, int nnodes, char *input_file)
{
    fprintf(log_file, "$STAT\tmod\tstart\tzbest\ttime_limit\tseed\tnnodes\tinput_file\n");
    fprintf(log_file, "$STAT\t%d\t%d\t%lf\t%lf\t%d\t%d\t%s\n",
            model_type,
            node_start,
            zbest,     // double
            timelimit, // double
            randomseed,
            nnodes,
            input_file);
}