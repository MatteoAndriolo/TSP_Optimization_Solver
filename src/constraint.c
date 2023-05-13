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
    DEBUG_COMMENT("constraint.c:add_degree_constraint", "NUMBER OF ROW IN CPLEX after adding degree constraints %d", CPXgetnumrows(env, lp));
    free(cname[0]);
    free(cname);
    free(index);
    free(value);
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
            print_error("CPXmipopt() error, not able to solve the problem");

        ncols = CPXgetnumcols(env, lp);
        double *xstar = (double *)malloc(ncols * sizeof(double));
        error = CPXgetx(env, lp, xstar, 0, ncols - 1);
        if (error)
            print_error("CPXgetx() error, not able to retrive the solution");
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
                print_error("CPXaddrows(): error 1");
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
