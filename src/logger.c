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

void logger_close()
{
    fclose(log_file);
}

void ffflush()
{
    fflush(log_file);
}

void log_path(const int *path, int nnodes)
{
    char str[10000];
    int i;
    for (i = 0; i < nnodes; i++) {
        sprintf(str+strlen(str), "%d,", path[i]);
    }
    DEBUG_COMMENT("log_path", "%s,",str);
}

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