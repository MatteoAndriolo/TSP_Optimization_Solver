#include "utilscplex.h"

void print_error(const char *err)
{
	DEBUG_COMMENT("utils.c:print_error", "\n\n ERROR: %s \n\n", err);
	printf("\n\n ERROR: %s \n\n", err);
	fflush(NULL);
	exit(1);
}

double dist(int i, int j, Instance *inst)
{
	double dx = inst->x[i] - inst->x[j];
	double dy = inst->y[i] - inst->y[j];
	if (!inst->integer_costs)
		return sqrt(dx * dx + dy * dy);
	int dis = sqrt(dx * dx + dy * dy) + 0.499999999;
	DEBUG_COMMENT("utils.c:dist", "i=%d, j=%d, dis=%d", i, j, dis);
	return dis + 0.0;
}

int xpos(int i, int j, Instance *inst)
{
	if (i == j)
		print_error(" i == j in xpos");
	if (i > j)
		return xpos(j, i, inst);
	int pos = i * inst->nnodes + j - ((i + 1) * (i + 2)) / 2;
	return pos;
}

void xstarToPath(Instance *inst, double *xstar, int dim_xstar, int *path)
{
	int *copy_path = (int *)calloc(inst->nnodes * 2, sizeof(int));
	int count = 0;
	for (int i = 0; i < inst->nnodes; i++)
	{
		for (int j = i + 1; j < inst->nnodes; j++)
		{
			if (xstar[xpos(i, j, inst)] > 0.5)
			{
				DEBUG_COMMENT("tspcplex.c:TSPopt", "x(%d,%d) = %f", i + 1, j + 1, xstar[xpos(i, j, inst)]);
				copy_path[count++] = i + 1;
				copy_path[count++] = j + 1;
			}
		}
	}

	for (int i = 0; i < inst->nnodes; i++)
		path[i] = 0;
	// log_path(copy_path, inst->nnodes * 2);

	path[0] = copy_path[0];
	DEBUG_COMMENT("utilscplex.c:xstarToPath", "path[%d] = %d", 0, path[0]);
	path[1] = copy_path[1];
	DEBUG_COMMENT("utilscplex.c:xstarToPath", "path[%d] = %d", 1, path[1]);
	copy_path[0] = -1;
	copy_path[1] = -1;
	for (int i = 2; i < inst->nnodes; i++) // for loop to write path
	{
		for (int j = 0; j < inst->nnodes * 2; j++)
		{
			if (path[i - 1] == copy_path[j])
			{
				if (j % 2 == 0)
				{
					path[i] = copy_path[j + 1];
					copy_path[j] = -1;
					copy_path[j + 1] = -1;
					break;
				}
				else
				{
					path[i] = copy_path[j - 1];
					copy_path[j] = -1;
					copy_path[j - 1] = -1;
					break;
				}
			}
		}

		DEBUG_COMMENT("utilscplex.c:xstarToPath", "path[%d] = %d", i, path[i]);
	}
	free(copy_path);
	for (int j = 0; j < inst->nnodes; j++)
		path[j]--;
#ifndef PRODUCTION
	char *tmp;
	tmp = getPath(path, inst->nnodes);
	DEBUG_COMMENT("utilscplex.c:xstarToPath", "path = %s", tmp);
	free(tmp);
#endif
}

void get_path_from_CPXIntSolution(const double *xstar, Instance *inst, int *succ, int *comp, int *ncomp)
{

	*ncomp = 0;
	for (int i = 0; i < inst->nnodes; i++)
	{
		succ[i] = -1;
		comp[i] = -1;
	}

	for (int start = 0; start < inst->nnodes; start++)
	{
		if (comp[start] >= 0)
			continue; // node "start" was already visited, just skip it

		// a new component is found
		(*ncomp)++;
		int i = start;
		int done = 0;
		while (!done) // go and visit the current component
		{
			comp[i] = *ncomp;
			done = 1;
			for (int j = 0; j < inst->nnodes; j++)
			{
				if (i != j && xstar[xpos(i, j, inst)] > 0.5 && comp[j] == -1) // the edge [i,j] is selected in xstar and j was not visited before
				{
					succ[i] = j;
					i = j;
					done = 0;
					break;
				}
			}
		}
		succ[i] = start; // last arc to close the cycle
						 // go to the next component...
		DEBUG_COMMENT("tspcplex.c:build_sol", "succ: %s", getPath(succ, inst->nnodes));
		DEBUG_COMMENT("tspcplex.c:build_sol", "comp: %s", getPath(comp, inst->nnodes));
	}
}