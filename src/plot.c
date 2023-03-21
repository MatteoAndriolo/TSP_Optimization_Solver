#include "plot.h"
#include "logger.h"
/*Use 2 different plot file for solution*/
void plot(const Instance *inst)
{
    DEBUG_COMMENT("plot::plot","open gnuplot pipe");
    FILE *gnuplotPipe = fopen("src/script.p", "w");

    if (!gnuplotPipe)
    {
        fprintf(stderr, "Error: could not open gnuplot pipe.\n");
        exit(1);
        ERROR_COMMENT("plot::plot","Error: could not open gnuplot pipe");
    }
    DEBUG_COMMENT("plot::plot","opened gnuplot pipe");

    // Send commands to gnuplot to create a 2D graph and plot the points
    fprintf(gnuplotPipe, "set title 'My Plot'\n");
    fprintf(gnuplotPipe, "set xlabel 'X Axis'\n");
    fprintf(gnuplotPipe, "set ylabel 'Y Axis'\n");
    fprintf(gnuplotPipe, "plot '-' with line, '-' with points\n");

    // Implicit draw the line
    int node;
    for (int i = 0; i < inst->nnodes; i++)
    {
        node=inst->path_best[i];
        fprintf(gnuplotPipe, "%d %d\n", (int)inst->x[node], (int)inst->y[node]);
    }
    fprintf(gnuplotPipe, "%d %d\n", (int)inst->x[inst->path_best[0]], (int)inst->y[inst->path_best[0]]);
    fprintf(gnuplotPipe, "e\n");
    for (int i = 0; i < inst->nnodes; i++)
    {
        node=inst->path_best[i];
        fprintf(gnuplotPipe, "%d %d\n", (int)inst->x[node], (int)inst->y[node]);
    }


    DEBUG_COMMENT("plot::plot","close gnuplot pipe");
    fclose(gnuplotPipe);
    DEBUG_COMMENT("plot::plot", "write on the terminal the command to launch gnuplot");
    printf("\n%i\n", system("gnuplot -p src/script.p"));
}
