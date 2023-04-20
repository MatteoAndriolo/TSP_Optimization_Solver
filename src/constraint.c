#include "constraint.h"

void add_degree_constraint(Instance *inst, CPXENVptr env, CPXLPptr lp)
{

    INFO_COMMENT("constraint.c:add_degree_constraint", "Adding degree constraints");
    char **cname = (char **)calloc(1, sizeof(char *));
    cname[0] = (char *)calloc(256, sizeof(char));

    int *index = (int *)calloc(inst->nnodes, sizeof(int));
    double *value = (double *)calloc(inst->nnodes, sizeof(double));
    DEBUG_COMMENT("constraint.c:add_degree_constraint", "NUMBER OF ROW IN CPLEX %d", CPXgetnumrows(env,lp));

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
            // DEBUG_COMMENT("constraint.c:add_degree_constraint", "index[%d] = %d", nnz, index[nnz]);
        }
        log_path(index, inst->nnodes-1);
        int izero = 0;
        if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, &cname[0]))
            print_error("CPXaddrows(): error 1");
    }
    DEBUG_COMMENT("constraint.c:add_degree_constraint", "NUMBER OF ROW IN CPLEX after adding degree constraints %d", CPXgetnumrows(env,lp));
    free(cname[0]);
    free(cname);
    free(index);
    free(value);
}

void add_subtour_constraints(Instance *inst, CPXENVptr env, CPXLPptr lp)
{
    int ncomp = 0;
    int n_STC = 0;
    int error;
    int ncols;
    //take the time and if exceed the time limit then break the loop
    time_t start_time = time(NULL);
    while (ncomp != 1 && difftime(time(NULL), start_time) < 100)
    {
        INFO_COMMENT("constraint.c:add_subtour_constraints", "starting the while loop");
        error = CPXmipopt(env, lp);
        CRITICAL_COMMENT("constraint.c:add_subtour_constraints", "0");
        if (error)
            print_error("CPXmipopt() error");
        ncols = CPXgetnumcols(env, lp);
        CRITICAL_COMMENT("constraint.c:add_subtour_constraints", "1");
        DEBUG_COMMENT("constraint.c:add_subtour_constraints", "num coulumn = %d", ncols);
        double *xstar = (double *)calloc(ncols, sizeof(double));
        error = CPXgetx(env, lp, xstar, 0, ncols - 1);
        if (error)
            print_error("CPXgetx() error");
        INFO_COMMENT("constraint.c:add_subtour_constraints", "created xstar");
        int *succ = (int *)malloc(sizeof(int) * inst->nnodes);
        int *comp = (int *)malloc(sizeof(int) * inst->nnodes);
        int *already_touched = (int *)calloc(inst->nnodes, sizeof(int));

        build_sol(xstar, inst, succ, comp, &ncomp);
        //FIXME does not produce correct comp
        DEBUG_COMMENT("constraint.c:add_subtour_constraint", "succ: %s",getPath(succ, inst->nnodes));
        DEBUG_COMMENT("constraint.c:add_subtour_constraint", "comp: %s",getPath(succ, inst->nnodes));

        for (int cc = 1; cc <= ncomp; cc++)
        {   
            char **cname = (char **)calloc(1, sizeof(char *));
            cname[0] = (char *)calloc(256, sizeof(char));
            sprintf(cname[0], "subtour(%d)", n_STC++);
            int *index = (int *)calloc(inst->nnodes, sizeof(int));
            double *value = (double *)calloc(inst->nnodes, sizeof(double));
            char sense = 'L'; // 'L' for less than or equal to constraint
            int nnz = 0;

            int j = 0;
            int a,b,k;
            while (j < inst->nnodes)
            {
                while (comp[j] != cc)
                {
                    j++;
                }
                k=j;
                while (k < inst->nnodes)
                {
                    if (comp[j] == cc)
                    {
                        a = j < succ[j] ? j : succ[j];  // FIXME need to put -1??
                        b = j >= succ[j] ? j : succ[j]; // FIXME need to put -1??
                        index[nnz] = xpos(a, b, inst);
                        value[nnz] = 1.0;
                        nnz++;
                    }
                    j++;
                }
            }

            // //log_path(index, inst->nnodes);
            // int *array_to_insert = (int *)calloc(inst->nnodes, sizeof(int));

            // int count = 0;
            // for (int i = 0; i < inst->nnodes; i++)
            // {
            //     if (index[i] > 0 )
            //     {
            //         array_to_insert[count] = index[i];
            //         value[count] = 1.0;
            //         already_touched[i] = 1;
            //         count++;
            //     }
            // }
            // log_path(array_to_insert, count);
            int izero = 0;
            //double rsh = count - 1;
            double rsh = nnz - 1;
            //if (CPXaddrows(env, lp, 0, 1, count + 1, &rsh, &sense, &izero, array_to_insert, value, NULL, &cname[0]))
            if (CPXaddrows(env, lp, 0, 1, rsh + 1, &rsh, &sense, &izero, index, value, NULL, &cname[0]))
                print_error("CPXaddrows(): error 1");
            free(cname[0]);
            free(cname);
            free(index);
            free(value);
            //free(array_to_insert);
            WARNING_COMMENT("constraint.c:add_subtour_constraints", "FREEING MEMORY, index, value, index1, cname[0], cname");
        }
        free(already_touched);
        free(xstar);
        free(succ);
        free(comp);
        WARNING_COMMENT("constraint.c:add_subtour_constraints", "FREEING MEMORY, xstar, succ, comp");
    }
}