#include "../include/utilscplex.h"

int xpos(int i, int j, Instance *inst)
{
	if (i == j)
        ERROR_COMMENT("utilscplex.c:xpos", "i == j");
	if (i > j)
		return xpos(j, i, inst);
	int pos = i * inst->nnodes + j - ((i + 1) * (i + 2)) / 2;
	return pos;
}

void xstarToPath(Instance *inst, double *xstar, int dim_xstar, int *path)
{
	DEBUG_COMMENT("utilscplex.c:xstarToPath", "xstarToPath");
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
    INSTANCE_pathCheckpoint(inst);
	free(copy_path);
	for (int j = 0; j < inst->nnodes; j++)
		path[j]--;
//#ifndef PRODUCTION
//	char *tmp;
//	tmp = getPath(path, inst->nnodes);
//	DEBUG_COMMENT("utilscplex.c:xstarToPath", "path = %s", tmp);
//	free(tmp);
//#endif
}

void create_xheu(Instance *inst, double *xheu)
{
	DEBUG_COMMENT("utilscplex.c:create_xheu", "Creating xheu");
	for (int i = 0; i < inst->nnodes - 1; i++) {
		xheu[xpos(inst->path[i], inst->path[i + 1], inst)] = 1.0;
    }
	inst->path[xpos(inst->path[inst->nnodes - 1], inst->path[0], inst)] = 1.0;
}

void generate_mip_start(Instance *inst, CPXENVptr env, CPXLPptr lp, double *xheu)
{
	INFO_COMMENT("utilscplex.c:ge*nerate_mip_start", "Generating MIP start");
	int effortlevel = CPX_MIPSTART_NOCHECK;
	int beg = 0;
	int *ind = (int *)calloc(inst->ncols, sizeof(int));
	for (int j = 0; j < inst->ncols; j++)
		ind[j] = j;
	if (CPXaddmipstarts(env, lp, 1, inst->ncols, &beg, ind, xheu, &effortlevel, NULL))
        ERROR_COMMENT("utilscplex.c:generate_mip_start", "CPXaddmipstarts() error");
	free(ind);
	INFO_COMMENT("utilscplex.c:generate_mip_start", "MIP start ENDED");
}

void fix_edges(CPXENVptr env, CPXLPptr lp, Instance *inst, double *xheu)
{
	DEBUG_COMMENT("utilscplex.c:fix_edges", "Fixing edges");
	int *ind = (int *)calloc(inst->nnodes, sizeof(int));
	double *bd = (double *)calloc(inst->nnodes, sizeof(double));
	for (int i = 0; i < inst->nnodes; i++)
	{
		for (int j = i + 1; j < inst->nnodes; j++)
		{
			if (xheu[xpos(i, j, inst)] > 0.5) // TODO check hyperparameter alpha 0.5 0.6 0.8
			{
				int random = rand() % 100;
				if (random < inst->percentageHF)
				{
					ind[i] = xpos(i, j, inst);
					bd[i] = 1.0;
					if (CPXchgbds(env, lp, 1, &ind[i], "L", &bd[i]))
                        ERROR_COMMENT("utilscplex.c:fix_edges", "CPXchgbds() error");
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
                ERROR_COMMENT("utilscplex.c:fix_edges", "CPXchgbds() error");
		}
	}
}

void eliminate_radius_edges(CPXENVptr env, CPXLPptr lp, Instance *inst, double *xheu, int radious)
{
	INFO_COMMENT("utilscplex.c:eliminate_fix_edges", "eliminating fixed edges form xheu for local branching");
	int *ind = (int *)calloc(inst->nnodes, sizeof(int));
	double *bd = (double *)calloc(inst->nnodes, sizeof(double));
	// radious should be expressed in a value between 0 and 1
	int percentage = (int)(inst->nnodes * radious);
	int count = 0;
	int flag = 0;
	while (count < percentage)
	{
		for (int i = 0; i < inst->nnodes; i++)
		{
			for (int j = 0; j < inst->nnodes; j++)
			{
				if (xheu[xpos(i, j, inst)] > 0.5 && (rand() % 100) < 50)
				{
					ind[i] = xpos(i, j, inst);
					bd[i] = 1.0;
					if (CPXchgbds(env, lp, 1, &ind[i], "L", &bd[i]))
                        ERROR_COMMENT("utilscplex.c:eliminate_fix_edges", "CPXchgbds() error");
					count++;
					if (count >= percentage)
					{
						flag = 1;
						break;
					}
				}
			}
			if (flag)
				break;
		}
		if (flag)
			break;
	}
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
			if (xheu[xpos(i, j, inst)] > 0.5)
			{
				ind[i] = xpos(i, j, inst);
				bd[i] = 0.0;
				if (CPXchgbds(env, lp, 1, &ind[i], "L", &bd[i]))
                    ERROR_COMMENT("utilscplex.c:repristinate_fix_edges", "CPXchgbds() error");
			}
		}
	}
}

int add_subtour_constraints(Instance *inst, CPXENVptr env, CPXLPptr lp)
{
    INFO_COMMENT("constraint.c:add_subtour_constraints", "Adding subtour constraints, starting the while loop for BENDERS");
    int ncomp = 0;
    int n_STC = 0;
    int error;
    int ncols;
    // take the time and if exceed the time limit then break the loop
    time_t start_time = time(NULL);
    while (ncomp != 1 && difftime(time(NULL), start_time) < 100)
    {
        error = CPXmipopt(env, lp);
        if (error)
            ERROR_COMMENT("constraint.c:add_subtour_constraints", "CPXmipopt() error, not able to solve the problem");

        ncols = CPXgetnumcols(env, lp);
        double *xstar = (double *)malloc(ncols * sizeof(double));
        error = CPXgetx(env, lp, xstar, 0, ncols - 1);
        if (error)
            ERROR_COMMENT("constraint.c:add_subtour_constraints", "CPXgetx() error, not able to retrive the solution");
        int *succ = (int *)malloc(sizeof(int) * inst->nnodes);
        int *comp = (int *)malloc(sizeof(int) * inst->nnodes);

        build_sol(xstar, inst, succ, comp, &ncomp);

        if (ncomp == 1)
            break;

        for (int cc = 1; cc <= ncomp; cc++)
        {
            char **cname = (char **)calloc(1, sizeof(char *));
            cname[0] = (char *)calloc(256, sizeof(char));
            sprintf(cname[0], "subtour(%d)", n_STC);
            int nncc = 0;
            for (int i = 0; i < inst->nnodes; i++)
            {
                if (comp[i] == cc)
                    nncc++;
            }

            int *index = (int *)calloc((inst->nnodes * (inst->nnodes - 1)) / 2, sizeof(int));
            double *value = (double *)calloc((inst->nnodes * (inst->nnodes - 1)) / 2, sizeof(double));
            char sense = 'L'; // 'L' for less than or equal to constraint
            int nnz = 0;

            int j = 0;
            int k;
            while (j < inst->nnodes - 1)
            {
                while (comp[j] != cc && j < inst->nnodes)
                {
                    j++;
                }
                k = j + 1;
                while (k < inst->nnodes)
                {
                    if (comp[k] == cc)
                    {
                        index[nnz] = xpos(j, k, inst);
                        value[nnz] = 1.0;
                        nnz++;
                    }
                    k++;
                }
                j++;
            }

            int izero = 0;
            double rsh = nncc - 1;

            if (CPXaddrows(env, lp, 0, 1, nnz, &rsh, &sense, &izero, index, value, NULL, &cname[0]))
                ERROR_COMMENT("constraint.c:add_subtour_constraints", "CPXaddrows(): error 1");
            DEBUG_COMMENT("constraint.c:add_subtour_constraints", "inserted row %d", CPXgetnumrows(env, lp));

            free(cname[0]);
            free(cname);
            free(index);
            free(value);
            // CPXwriteprob(env, lp, "mipopt.lp", NULL);
        }
        build_sol(xstar, inst, succ, comp, &ncomp);
        free(xstar);
        free(succ);
        free(comp);
    }
    return 0;
}

void build_sol(const double *xstar, Instance *inst, int *succ, int *comp, int *ncomp)
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
        // DEBUG_COMMENT("tspcplex.c:build_model", "succ: %s", getPath(succ, inst->nnodes));
        // DEBUG_COMMENT("tspcplex.c:build_model", "comp: %s", getPath(comp, inst->nnodes));
    }
}
