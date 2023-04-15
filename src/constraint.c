#include "constraint.h"

void add_degree_constraint(Instance *inst, CPXENVptr env, CPXLPptr lp)
{

    INFO_COMMENT("constraint.c:add_degree_constraint", "Adding degree constraints");
    char **cname = (char **)calloc(1, sizeof(char *));
    cname[0] = (char *)calloc(256, sizeof(char));

    int *index = (int *)calloc(inst->nnodes, sizeof(int));
    double *value = (double *)calloc(inst->nnodes, sizeof(double));

    for (int h = 0; h < inst->nnodes; h++)
    {
        double rhs = 2.0;
        char sense = 'E'; // 'E' for equality constraint
        sprintf(cname[0], "degree(%d)", h + 1);
        int nnz = 0;
        for (int i = 0; i < inst->nnodes; i++)
        {
            if (i == h)
                continue;
            index[nnz] = xpos(i, h, inst);
            value[nnz] = 1.0;
            nnz++;
        }
        int izero = 0;
        if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, &cname[0]))
            print_error("CPXaddrows(): error 1");
    }

    free(cname[0]);
    free(cname);
    free(index);
    free(value);
}

void add_subtour_constraints(Instance *inst, CPXENVptr env, CPXLPptr lp)
{
    int ncomp = 0;
    int error;
    int ncols;
    while (ncomp != 1)
    {
        INFO_COMMENT("constraint.c:add_subtour_constraints", "starting the while loop");
        error = CPXmipopt(env, lp);
        CRITICAL_COMMENT("constraint.c:add_subtour_constraints", "0");
        if (error)
            print_error("CPXmipopt() error");
        ncols = CPXgetnumcols(env, lp);
        CRITICAL_COMMENT("constraint.c:add_subtour_constraints", "1");
        double *xstar = (double *)calloc(ncols, sizeof(double));
        error = CPXgetx(env, lp, xstar, 0, ncols - 1);
        if (error)
            print_error("CPXgetx() error");
        INFO_COMMENT("constraint.c:add_subtour_constraints", "created xstar");
        ncomp = 0;
        int *succ = (int *)malloc(sizeof(int) * inst->nnodes);
        int *comp = (int *)malloc(sizeof(int) * inst->nnodes);
        for (int i = 0; i < inst->nnodes; i++)
        {
            succ[i] = -1; // initialize the successor vector
            comp[i] = -1; // intialize the component vector
        }
        for (int start = 0; start < inst->nnodes; start++)
        {
            if (comp[start] >= 0)
                continue; // node "start" was already visited, just skip it

            // a new component is found
            (ncomp)++;
            int i = start;
            int done = 0;
            while (!done) // go and visit the current component
            {
                comp[i] = ncomp;
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
            // add this connecte component as a subtour elimination constraint
            char *cname = (char *)calloc(1, sizeof(char));
            cname[0] = (char *)calloc(256, sizeof(char));
            sprintf(cname[0], "subtour(%d)", ncomp);
            int *index = (int *)calloc(inst->nnodes, sizeof(int));
            double *value = (double *)calloc(inst->nnodes, sizeof(double));
            double rhs = inst->nnodes - 1;
            char sense = 'L'; // 'L' for less than or equal to constraint
            int nnz = 0;
            for (int i = 0; i < inst->nnodes; i++)
            {
                if (i == start)
                    continue;
                index[nnz] = xpos(i, succ[i], inst);
                value[nnz] = 1.0;
                nnz++;
            }
            int izero = 0;
            if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, &cname[0]))
                print_error("CPXaddrows(): error 1");
            free(cname[0]);
            free(cname);
            free(index);
            free(value);
            DEBUG_COMMENT("constraint.c:add_subtour_constraints", "subtour constraint added for component %d", ncomp);
        }
        free(xstar);
        free(succ);
        free(comp);
        INFO_COMMENT("constraint.c:add_subtour_constraints", "freeing memory");
    }
}
