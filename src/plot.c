#include "plot.h"

void plot(Instance *inst){
	if (inst->verbosity >= 2){
		printf("... PLOTTING with gnu plot");
	}
	FILE* gnuplotPipe = fopen("script.p", "w");

	if (!gnuplotPipe) {
        fprintf(stderr, "Error: could not open gnuplot pipe.\n");
        exit(1);
    }

    // Send commands to gnuplot to create a 2D graph and plot the points
    fprintf(gnuplotPipe, "set title 'My Plot'\n");
    fprintf(gnuplotPipe, "set xlabel 'X Axis'\n");
    fprintf(gnuplotPipe, "set ylabel 'Y Axis'\n");
    fprintf(gnuplotPipe, "plot '-'  with line\n");
	
	// Implicit draw the line 
    for (int i = 0; i < inst->nnodes; i++) {
        fprintf(gnuplotPipe, "%lf %lf\n", inst->x[i], inst->y[i]);
    }

	fclose(gnuplotPipe);

	printf("%i", system("gnuplot -p script.p"));
}
