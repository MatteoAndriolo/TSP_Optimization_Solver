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

void create_xheu(Instance *inst, double *xheu, int *path)
{
	INFO_COMMENT("utilscplex.c:create_xheu", "Creating xheu");
	for (int i = 0; i < inst->nnodes - 1; i++)
		xheu[xpos(path[i], path[i + 1], inst)] = 1.0;
	xheu[xpos(path[inst->nnodes - 1], path[0], inst)] = 1.0;
}

void generate_mip_start(Instance *inst, CPXENVptr env, CPXLPptr lp, double *xheu)
{
	INFO_COMMENT("utilscplex.c:generate_mip_start", "Generating MIP start");
	int effortlevel = CPX_MIPSTART_NOCHECK;
	int beg = 0;
	int *ind = (int *)calloc(inst->ncols, sizeof(int));
	for (int j = 0; j < inst->ncols; j++)
		ind[j] = j;
	if (CPXaddmipstarts(env, lp, 1, inst->ncols, &beg, ind, xheu, &effortlevel, NULL))
		print_error("CPXaddmipstarts() error");
	free(ind);
	INFO_COMMENT("utilscplex.c:generate_mip_start", "MIP start ENDED");
}

void fix_edges(CPXENVptr env, CPXLPptr lp, Instance *inst, double *xheu)
{
	INFO_COMMENT("utilscplex.c:fix_edges", "Fixing edges");
	int *ind = (int *)calloc(inst->nnodes, sizeof(int));
	double *bd = (double *)calloc(inst->nnodes, sizeof(double));
	for (int i = 0; i < inst->nnodes; i++)
	{
		for (int j = i + 1; j < inst->nnodes; j++)
		{
			if (xheu[xpos(i, j, inst)] > 0.5)
			{
				int random = rand() % 100;
				if (random < 80)
				{
					ind[i] = xpos(i, j, inst);
					bd[i] = 1.0;
					if (CPXchgbds(env, lp, 1, &ind[i], "L", &bd[i]))
						print_error("CPXchgbds() error");
				}
			}
		}
	}
}
void unfix_edges(CPXENVptr env, CPXLPptr lp, Instance *inst, double *xheu)
{
	INFO_COMMENT("utilscplex.c:fix_edges", "unfixing edges");
	int *ind = (int *)calloc(inst->nnodes, sizeof(int));
	double *bd = (double *)calloc(inst->nnodes, sizeof(double));
	for (int i = 0; i < inst->nnodes; i++)
	{
		for (int j = i + 1; j < inst->nnodes; j++)
		{
			ind[i] = xpos(i, j, inst);
			bd[i] = 0.0;
			if (CPXchgbds(env, lp, 1, &ind[i], "L", &bd[i]))
				print_error("CPXchgbds() error");
		}
	}
}

void eliminate_radius_edges(CPXENVptr env, CPXLPptr lp, Instance *inst, double *xheu, int radious)
{
	INFO_COMMENT("utilscplex.c:eliminate_fix_edges", "eliminating fixed edges form xheu for local branching");
	int sum_preserved_edges = 0;
	int max_edge_find = 0;
	int min_edge_find = 10000000;
	double *xheu_copy = (double *)calloc(inst->ncols, sizeof(double));
	for (int i = 0; i < inst->ncols; i++)
		xheu_copy[i] = xheu[i];

	for (int i = 0; i < inst->nnodes; i++)
	{
		for (int j = i + 1; j < inst->nnodes; j++)
		{
			if (xheu[xpos(i, j, inst)] > 0.5)
			{
				sum_preserved_edges++;
				if (max_edge_find < dist(i, j, inst))
					max_edge_find = dist(i, j, inst);
				if (min_edge_find > dist(i, j, inst))
					min_edge_find = dist(i, j, inst);
			}
		}
	}
	int K = sum_preserved_edges - radious;
	DEBUG_COMMENT("utilscplex.c:eliminate_fix_edges", "sum_preserved_edges = %d, max_edge_find = %d, min_edge_find = %d, K = %d", sum_preserved_edges, max_edge_find, min_edge_find, K);
	int count = 0;
	int flag = 0;
	while (count != sum_preserved_edges - K)
	{
		int *ind = (int *)calloc(inst->nnodes, sizeof(int));
		double *bd = (double *)calloc(inst->nnodes, sizeof(double));
		for (int i = 0; i < inst->nnodes; i++)
		{
			for (int j = i + 1; j < inst->nnodes; j++)
			{
				if (xheu_copy[xpos(i, j, inst)] > 0.5)
				{
					xheu_copy[xpos(i, j, inst)] = 0.0;
					int distance_normalized = (dist(i, j, inst) - min_edge_find) * 100 / (max_edge_find - min_edge_find);
					int random = rand() % (100 - count);
					if (random < distance_normalized)
					{
						ind[i] = xpos(i, j, inst);
						bd[i] = 0.0;
						if (CPXchgbds(env, lp, 1, &ind[i], "U", &bd[i]))
							print_error("CPXchgbds() error");
						if (count == sum_preserved_edges - K)
						{
							flag = 1;
							break;
						}
						count++;
					}
				}
			}
			if (flag == 1)
				break;
		}
		if (flag == 1)
			break;
	}
	free(xheu_copy);
}

void repristinate_radius_edges(CPXENVptr env, CPXLPptr lp, Instance *inst, double *xheu)
{
	INFO_COMMENT("utilscplex.c:repristinate_fix_edges", "repristinating fixed edges form xheu for local branching");
	int *ind = (int *)calloc(inst->nnodes, sizeof(int));
	double *bd = (double *)calloc(inst->nnodes, sizeof(double));
	for (int i = 0; i < inst->nnodes; i++)
	{
		for (int j = i + 1; j < inst->nnodes; j++)
		{
			ind[i] = xpos(i, j, inst);
			bd[i] = 1.0;
			if (CPXchgbds(env, lp, 1, &ind[i], "U", &bd[i]))
				print_error("CPXchgbds() error");
		}
	}
}