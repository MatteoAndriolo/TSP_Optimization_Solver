#include "plot.h"
#include "logger.h"

void plot(const int *path, const double *x, const double *y, const int nnodes, const char *title)
{
    DEBUG_COMMENT("plot::plot", "open gnuplot pipe");
    FILE *gnuplotPipe = fopen("src/script.p", "w");

    if (!gnuplotPipe)
    {
        fprintf(stderr, "Error: could not open gnuplot pipe.\n");
        exit(1);
        ERROR_COMMENT("plot::plot", "Error: could not open gnuplot pipe");
    }
    DEBUG_COMMENT("plot::plot", "opened gnuplot pipe");

    // Send commands to gnuplot to create a 2D graph and plot the points
    fprintf(gnuplotPipe, "set title '%s'\n", title);
    fprintf(gnuplotPipe, "set xlabel 'X Axis'\n");
    fprintf(gnuplotPipe, "set ylabel 'Y Axis'\n");
    fprintf(gnuplotPipe, "plot '-' with line, '-' with points\n");

    // Implicit draw the line
    int node;
    for (int i = 0; i < nnodes; i++)
    {
        node = path[i];
        fprintf(gnuplotPipe, "%d %d\n", (int)x[node], (int)y[node]);
    }
    fprintf(gnuplotPipe, "%d %d\n", (int)x[path[0]], (int)y[path[0]]);
    fprintf(gnuplotPipe, "e\n");
    for (int i = 0; i < nnodes; i++)
    {
        node = path[i];
        fprintf(gnuplotPipe, "%d %d\n", (int)x[node], (int)y[node]);
    }
    fprintf(gnuplotPipe, "e\n");

    // PNG
    // fprintf(gnuplotPipe, "set terminal pngcairo size 800,600 font 'Arial,12'\n");
    // fprintf(gnuplotPipe, "set output 'plot/%s.png'\n", title);
    // SVG
    fprintf(gnuplotPipe, "set terminal svg size 800,600 font 'Arial,12'\n");
    fprintf(gnuplotPipe, "set output 'plot/%s.svg'\n", title);
    // PDF
    // fprintf(gnuplotPipe, "set terminal pdf size 800,600 font 'Arial,12'\n");
    // fprintf(gnuplotPipe, "set output 'plot/%s.pdf'\n", title);

    fprintf(gnuplotPipe, "replot");

    DEBUG_COMMENT("plot::plot", "close gnuplot pipe");
    fclose(gnuplotPipe);
    DEBUG_COMMENT("plot::plot", "write on the terminal the command to launch gnuplot");
    printf("\n%i\n", system("gnuplot -p src/script.p"));
}
