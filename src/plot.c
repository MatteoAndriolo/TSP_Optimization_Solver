#include "plot.h"
#include "logger.h"
/*Use 2 different plot file for solution*/
void plot(Instance *inst)
{
    DEBUG_COMMENT("plot::plot","open gnuplot pipe");
    FILE *gnuplotPipe = fopen("src/script.p", "w");

    if (!gnuplotPipe)
    {
        fprintf(stderr, "Error: could not open gnuplot pipe.\n");
        exit(1);
        ERROR_COMMENT("plot::plot","Error: could not open gnuplot pipe");
    }

    // Send commands to gnuplot to create a 2D graph and plot the points
    fprintf(gnuplotPipe, "set title 'My Plot'\n");
    fprintf(gnuplotPipe, "set xlabel 'X Axis'\n");
    fprintf(gnuplotPipe, "set ylabel 'Y Axis'\n");
    fprintf(gnuplotPipe, "plot '-' with point, '-' with line\n");

    // TODO replace solution with model solution
    int dim_solution = 9;
    int solution[] = {1, 45, 3, 7, 9, 12, 15, 18, 1};
    
    // Implicit draw the line
    for (int i = 0; i < inst->nnodes; i++)
    {
        int found = 0; 
        for (int j = 0; j < dim_solution; j++)
        {
            if (i == solution[j])
            {
                found = 1;
                break;
            }
        }
        if (!found)
            fprintf(gnuplotPipe, "%d %d\n", (int)inst->x[i], (int)inst->y[i]);
    }

    fprintf(gnuplotPipe, "e\n");

    for (int i = 0; i < dim_solution; i++)
    {
        int nn = solution[i];
        fprintf(gnuplotPipe, "%d %d\n", (int)inst->x[nn], (int)inst->y[nn]);
    }

    fprintf(gnuplotPipe, "e\n");
    DEBUG_COMMENT("plot::plot","close gnuplot pipe");
    fclose(gnuplotPipe);
    DEBUG_COMMENT("plot::plot", "write on the terminal the command to launch gnuplot");
    printf("\n%i\n", system("gnuplot -p src/script.p"));
}
