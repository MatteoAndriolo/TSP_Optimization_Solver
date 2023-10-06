#include "../include/tspcplex.h"
#include <stdbool.h>
#include <string.h>
#define EPS 1e-5

void build_model(Instance *inst, CPXENVptr env, CPXLPptr lp) {
    INFO_COMMENT("tspcplex:build_model", "Building model");
    // double zero = 0.0;

    char **cname =
        (char **)calloc(1, sizeof(char *));  // (char **) required by cplex...
    cname[0] = (char *)calloc(100, sizeof(char));
    inst->ncols=0;
    inst->edgeList =
        (double *)calloc((inst->nnodes * (inst->nnodes - 1)) / 2, sizeof(double));

    // ADD PARAMETERS - COLUMNS
    // add binary var.s x(i,j) for i < j
    for (int i = 0; i < inst->nnodes; i++) {
        for (int j = i + 1; j < inst->nnodes; j++) {
            double obj = INSTANCE_getDistanceNodes(inst, i, j);  // cost == distance
            double lb = 0.0;  // lower bound
            double ub = 1.0;  // upper bound
            char xctype = CPX_BINARY;
            sprintf(cname[0], "x(%d,%d)", i + 1,
                    j + 1);  // (i+1,j+1) because CPLEX starts from 1 (not 0)
            inst->edgeList[inst->ncols++] = obj;

            // eviroment , linear programming, cost of distance of x(i,j), lower
            // bound, upper bound, xctype, name of x(i,j)
            if (CPXnewcols(env, lp, 1, &obj, &lb, &ub, &xctype, cname))
                ERROR_COMMENT("tspcplex::build_model", " wrong CPXnewcols on x var.s");

            if (CPXgetnumcols(env, lp) - 1 != xpos(i, j, inst))
                ERROR_COMMENT("tspcplex::build_model", " wrong position for x var.s");
            DEBUG_COMMENT("tspcplex::build_model", "size edge list", inst->ncols);
        }
    }

    DEBUG_COMMENT("tspcplex:build_model",
            "number of columns in CPLEX after adding x var.s %d",
            CPXgetnumcols(env, lp));
    if (inst->ncols != CPXgetnumcols(env, lp))
        ERROR_COMMENT("tspcplex:build_model",
                " wrong number of columns in CPLEX after adding x var.s %d vs %d", inst->ncols, CPXgetnumcols(env, lp));
    if (inst->ncols != (inst->nnodes * (inst->nnodes - 1)) / 2)
        ERROR_COMMENT("tspcplex:build_model",
                " wrong number of columns in CPLEX after adding x var.s");

    // ADD CONSTRAITS - ROWS
    // add_degree_constraint(inst, env, lp) for each node
    INFO_COMMENT("constraint.c:add_degree_constraint",
            "Adding degree constraints");
    int *index = (int *)calloc(inst->nnodes, sizeof(int));
    double *value = (double *)calloc(inst->nnodes, sizeof(double));

    for (int i = 0; i < inst->nnodes; i++) {
        double rhs = 2.0;
        char sense = 'E';                        // 'E' for equality constraint
        sprintf(cname[0], "degree(%d)", i + 1);  // rowname
        int nnz = 0;  // number of non zero constraints to be added to constraint
                      // matrix length of array rmatind rmatval
        for (int j = 0; j < inst->nnodes; j++) {
            if (j == i) continue;
            index[nnz] = xpos(j, i, inst);  // rmatind
            value[nnz] = 1.0;               // rmatval
            nnz++;
        }
        int izero = 0;  // rmatbeg
                        // int  CPXaddrows( CPXCENVptr env, CPXLPptr lp, int ccnt, int rcnt, int
                        // nzcnt, double const * rhs, char const * sense, int const * rmatbeg, int
                        // const * rmatind, double const * rmatval, char ** colname, char ** rowname
                        // )
        if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL,
                    &cname[0]))
            ERROR_COMMENT("constraint.c:add_degree_constraint",
                    "CPXaddrows(): error 1");
    }
    DEBUG_COMMENT("constraint.c:add_degree_constraint",
            "NUMBER OF ROW IN CPLEX after adding degree constraints %d",
            CPXgetnumrows(env, lp));
    free(cname[0]);
    free(cname);
    free(index);
    free(value);
}

int doit_fn_concorde(double cutval, int cutcount, int *cut, void *inparam) {
    // Input *param = (Input *)inparam;
    Params_CC *param = (Params_CC *)inparam;
    Instance *inst = param->inst;
    CPXCALLBACKCONTEXTptr context = param->context;

    int ecount = inst->nnodes * (inst->nnodes - 1) / 2;
    double *value = (double *)calloc(ecount, sizeof(double));
    int *index = (int *)calloc(ecount, sizeof(int));
    char sense = 'L';
    double rhs = cutcount - 1;
    int purgeable = CPX_USECUT_FILTER;
    int local = 0;
    int izero = 0;
    int nnz = 0;
    for (int ipos = 0; ipos < cutcount; ipos++) {
        for (int jpos = ipos + 1; jpos < cutcount; jpos++) {
            index[nnz] = xpos(cut[ipos], cut[jpos], inst);
            value[nnz] = 1.0;
            nnz++;
        }
        // TODO: go single thread clone the model env(env2, lp2) put the poiters in
        // the instance data structure when I am in a callback add subtour
        // elimination constrain addrowsfunction and print the model
    }
    if (CPXcallbackaddusercuts(context, 1, nnz, &rhs, &sense, &izero, index,
                value, &purgeable, &local))
        ERROR_COMMENT("tspcplex.c:doit_fn_concorde",
                "CPXcallbackaddusercuts() error");
    printf("rhs = %10.0f, nnz = %2d, cutcount = %d \n", rhs, nnz, cutcount);
    free(value);
    free(index);
    return 0;
}

int my_callback_relaxation(CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
        void *userhandle) {
    printf("my_callback_relaxation\n");
    INFO_COMMENT("tspcplex.c:my_callback_relaxation",
            "entering relaxation callbacks");
    Instance *inst = (Instance *)userhandle;
    double *xstar = (double *)malloc(sizeof(double) * inst->ncols);
    double objval = CPX_INFBOUND;
    int ncomp = 0;
    int *comps = (int *)calloc(
            inst->nnodes, sizeof(int));  // edges pertaining to each component
    int *compscount = (int *)calloc(
            inst->nnodes, sizeof(int));  // number of nodes in each component
    int *elist = (int *)calloc(inst->ncols * 2, sizeof(int));  // list of edges
    int loader = 0;
    int ecount = 0;
    for (int i = 0; i < inst->nnodes; i++) {
        for (int j = i + 1; j < inst->nnodes; j++) {
            // TODO:check with xpos
            elist[loader++] = i;
            elist[loader++] = j;
            ecount++;
        }
    }
    //----------------------check if the solution work and che some
    //information--------------------------------
    if (CPXcallbackgetrelaxationpoint(context, xstar, 0, inst->ncols - 1,
                &objval))
        ERROR_COMMENT("tspcplex.c:my_callback_relaxation",
                "CPXcallbackgetrelaxationpoint error");
    int mythread = -1;
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread);
    int mynode = -1;
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode);
    double incumbent = CPX_INFBOUND;
    CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);
    //------------------------start using
    //concorde------------------------------------------------------------
    INFO_COMMENT("tspcplex.c:my_callback_relaxation",
            "calling CCcut_connect_components");
    if (CCcut_connect_components(inst->nnodes, ecount, elist, xstar, &ncomp,
                &compscount, &comps))
        ERROR_COMMENT("tspcplex.c:my_callback_relaxation",
                "CCcut_connect_components error");
    printf("ncomp = %d\n", ncomp);
    if (ncomp == 1)  // TODO separate components here
    {
        DEBUG_COMMENT("tspcplex.c:my_callback_relaxation",
                "inside the if condition on the relaxation point, we are in "
                "cause the number of the connected component, ncomp = %d",
                ncomp);
        if (CCcut_violated_cuts(inst->nnodes, ecount, elist, xstar, 2.0 - EPSILON,
                    doit_fn_concorde, context))
            ERROR_COMMENT("tspcplex.c:my_callback_relaxation",
                    "CCcut_violated_cuts error");
    }
    // TODO: check fuction for != ncomp
    free(xstar);
    free(comps);
    free(compscount);
    free(elist);
    return 0;
}

int my_callback_candidate(CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
        void *userhandle) {
    Instance *inst = (Instance *)userhandle;
    int *succ = (int *)malloc(sizeof(int) * inst->nnodes);
    int *comp = (int *)malloc(sizeof(int) * inst->nnodes);
    double *xstar = (double *)malloc(sizeof(double) * inst->ncols);
    double objval = CPX_INFBOUND;
    int ncomp;
    //----------------------check if the solution work and che some
    //information--------------------------------
    if (CPXcallbackgetcandidatepoint(context, xstar, 0, inst->ncols - 1, &objval))
        ERROR_COMMENT("tspcplex.c:my_callback_candidate",
                "CPXcallbackgetcandidatepoint error");
    int mythread = -1;
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread);
    int mynode = -1;
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode);
    double incumbent = CPX_INFBOUND;
    CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);
    //-----------------------buil the
    //solution-----------------------------------------------------------------
    build_sol(xstar, inst, succ, comp, &ncomp);
    //--------------------------------add the sec's
    //cut--------------------------------------------------------
    if (ncomp > 1) {
        for (int cc = 1; cc <= ncomp; cc++) {
            int nncc = 0;
            for (int i = 0; i < inst->nnodes; i++) {
                if (comp[i] == cc) nncc++;
            }

            int *index =
                (int *)calloc((inst->nnodes * (inst->nnodes - 1)) / 2, sizeof(int));
            double *value = (double *)calloc((inst->nnodes * (inst->nnodes - 1)) / 2,
                    sizeof(double));
            char sense = 'L';  // 'L' for less than or equal to constraint
            int nnz = 0;

            int j = 0;
            int k;
            while (j < inst->nnodes - 1) {
                while (comp[j] != cc && j < inst->nnodes) {
                    j++;
                }
                k = j + 1;
                while (k < inst->nnodes) {
                    if (comp[k] == cc) {
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
            if (CPXcallbackrejectcandidate(context, 1, nnz, &rsh, &sense, &izero,
                        index, value))
                ERROR_COMMENT("constraint.c:my_callback", "CPXaddrows(): error 1");
            free(index);
            free(value);
        }
    }
    free(succ);
    free(comp);
    free(xstar);
    return 0;
}

//-----correspond to sec_callback---------
int CPXPUBLIC my_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
        void *userhandle) {
    INFO_COMMENT("tspcplex.c:my_callback",
            "entering callbacks function here we are going to decide ");
    Instance *inst = (Instance *)userhandle;

    if (contextid == CPX_CALLBACKCONTEXT_RELAXATION)
        return my_callback_relaxation(context, contextid, inst);
    if (contextid == CPX_CALLBACKCONTEXT_CANDIDATE)
        return my_callback_candidate(context, contextid, inst);

    return 0;
}

int branch_and_cut(Instance *inst, CPXENVptr env, CPXLPptr lp,
        CPXLONG contextid) {
    if (CPXcallbacksetfunc(env, lp, contextid, my_callback, inst))
        ERROR_COMMENT("tspcplex.c:branch_and_cut", "CPXcallbacksetfunc() error");

    CPXmipopt(env, lp);

    double *xstar = (double *)calloc(inst->ncols, sizeof(double));
    CPXgetx(env, lp, xstar, 0, inst->ncols - 1);

    int *succ = (int *)calloc(inst->nnodes, sizeof(int));
    int *comp = (int *)calloc(inst->nnodes, sizeof(int));
    int ncomp;

    build_sol(xstar, inst, succ, comp, &ncomp);

    double z;
    int error = CPXgetobjval(env, lp, &z);
    if (error)
        ERROR_COMMENT("tspcplex.c:branch_and_cut", "CPXgetobjval() error\n");

    free(comp);
    free(succ);
    free(xstar);

    return 0;
}

int hard_fixing(CPXENVptr env, CPXLPptr lp, Instance *inst) {
    DEBUG_COMMENT("tspcplex.c:hard_fixing", "entering hard fixing");
    //----------------------------------------- set the time limit to run cplex
    //---------------------------------------------------------
    // TODO: create a function set the param of cplex
    CPXsetdblparam(env, CPXPARAM_TimeLimit, 15);
    //----------------------------------------- create the xheuristic soltion from
    //a warm start ----------------------------------------
    double *xheu = (double *)calloc(inst->ncols, sizeof(double));
    create_xheu(inst, xheu);
    generate_mip_start(inst, env, lp, xheu);

    //double best_solution = INFTY;
    bool no_improv = false;
    while (!no_improv) {
        //----------------------------------------- fixing some edges
        //---------------------------------------------------------------
        fix_edges(env, lp, inst, xheu);
#ifndef PRODUCTION
        CPXwriteprob(env, lp, "mipopt.lp", NULL);
#endif
        //--------------------------------- calling the branch and cut solution
        //-------------------------------------------------------
        inst->solver = 2;
        int status = solve_problem(
                env, lp,
                inst);  // branch_and_cut(inst, env, lp, CPX_CALLBACKCONTEXT_CANDIDATE |
                        // CPX_CALLBACKCONTEXT_RELAXATION);
        if (status) ERROR_COMMENT("tspcplex.c:hard_fixing", "Execution FAILED");
        // ---------------------------------- check if the solution is better than
        // the previous one ---------------------------------
        double actual_solution;
        int error = CPXgetobjval(env, lp, &actual_solution);
        if (error)
            ERROR_COMMENT("tspcplex.c:hard_fixing", "CPXgetobjval() error\n");

        if (actual_solution < inst->best_tourlength) {  // UPDATE path
            if (CPXgetx(env, lp, xheu, 0, inst->ncols - 1))
                ERROR_COMMENT("tspcplex.c:hard_fixing", "CPXgetx() error");
            inst->best_tourlength = actual_solution;
            xstarToPath(inst, xheu, inst->ncols, inst->path);
            memcpy(inst->best_path, inst->path, sizeof(int)*inst->ncols);
        }
        //----------------------------------------- unfixing  edges
        //---------------------------------------------------------------
        unfix_edges(env, lp, inst, xheu);

        if (actual_solution >= inst->best_tourlength) break;
    }

    inst->solver = 2;
    solve_problem(env, lp, inst);
    if (CPXgetx(env, lp, xheu, 0, inst->ncols - 1))
        ERROR_COMMENT("tspcplex.c:hard_fixing", "CPXgetx() error");
    xstarToPath(inst, xheu, inst->ncols, inst->path);

    free(xheu);
    return 0;
}


int base_cplex(CPXENVptr env, CPXLPptr lp, Instance *inst){
    CPXsetdblparam(env, CPXPARAM_TimeLimit, 10);
    double *xheu = (double *)calloc(inst->ncols, sizeof(double));
    create_xheu(inst, xheu);
    generate_mip_start(inst, env, lp, xheu);

    CPXmipopt(env, lp);

    double *xstar = (double *)calloc(inst->ncols, sizeof(double));
    CPXgetx(env, lp, xstar, 0, inst->ncols - 1);

    int *succ = (int *)calloc(inst->nnodes, sizeof(int));
    int *comp = (int *)calloc(inst->nnodes, sizeof(int));
    int ncomp;
    build_sol(xstar, inst, succ, comp, &ncomp);
    xstarToPath(inst, xstar, inst->ncols, inst->path);


    double z;
    int error = CPXgetobjval(env, lp, &z);
    if (error)
        ERROR_COMMENT("tspcplex.c:branch_and_cut", "CPXgetobjval() error\n");
    return SUCCESS;
}

int local_branching(CPXENVptr env, CPXLPptr lp, Instance *inst) {
    //----------------------------------------- set the time limit to run cplex
    //---------------------------------------------------------
    // TODO: create a function set the param of cplex
    CPXsetdblparam(env, CPXPARAM_TimeLimit, 15);
    //----------------------------------------- create the xheuristic soltion from
    //a warm start ----------------------------------------
    double *xheu = (double *)calloc(inst->ncols, sizeof(double));
    create_xheu(inst, xheu);
    generate_mip_start(inst, env, lp, xheu);

    double best_solution = INFTY;
    int no_improv = 0;
    while (no_improv < 1) {
        //------------------------------------------ take current solution and
        //eliminate some solutions -------------------------------
        eliminate_radius_edges(env, lp, inst, xheu, (double)0.90);
#ifndef PRODUCTION
        CPXwriteprob(env, lp, "mipopt.lp", NULL);
#endif
        //--------------------------------- calling the branch and cut solution
        //-------------------------------------------------------
        inst->solver = 2;
        int status = solve_problem(
                env, lp,
                inst);  // branch_and_cut(inst, env, lp, CPX_CALLBACKCONTEXT_CANDIDATE |
                        // CPX_CALLBACKCONTEXT_RELAXATION);
        if (status) ERROR_COMMENT("tspcplex.c:local_branching", "Execution FAILED");
        // ---------------------------------- check if the solution is better than
        // the previous one ---------------------------------
        double actual_solution;
        int error = CPXgetobjval(env, lp, &actual_solution);
        if (error)
            ERROR_COMMENT("tspcplex.c:local_branching", "CPXgetobjval() error\n");

        if (actual_solution < best_solution) {  // UPDATE path
            if (CPXgetx(env, lp, xheu, 0, inst->ncols - 1))
                ERROR_COMMENT("tspcplex.c:local_branching", "CPXgetx() error");
            best_solution = actual_solution;
            xstarToPath(inst, xheu, inst->ncols, inst->path);
        }
        //---------------------------------- Unfix the edges and calling thesolution
        //--------------------------------------------------
        repristinate_radius_edges(
                env, lp, inst,
                xheu);  // FIXME remove fixing the boundary and add a contraint row
        if (actual_solution >= best_solution) break;
    }
    //--------------------------------- calling the branch and cut solution
    //-------------------------------------------------------
    inst->solver = 2;
    solve_problem(env, lp, inst);
    if (CPXgetx(env, lp, xheu, 0, inst->ncols - 1))
        ERROR_COMMENT("tspcplex.c:local_branching", "CPXgetx() error");
    xstarToPath(inst, xheu, inst->ncols, inst->path);
    free(xheu);

    return 0;
}

int solve_problem(CPXENVptr env, CPXLPptr lp, Instance *inst) {
    DEBUG_COMMENT("tspcplex.c:solve_problem", "entering solve problem");
    int status = SUCCESS;
    printf(
            "////////////////////////////////////////////////////////////////////////"
            "/////////\n");
    printf("solver = %d\n", inst->solver);
    printf(
            "////////////////////////////////////////////////////////////////////////"
            "/////////\n");
    CPXwriteprob(env, lp, "mipopt_base.lp", NULL);

    //    SOLVER_PATCHING_HEURISTIC = 2,
    //    SOLVER_BRANCH_AND_CUT=3,
    //    SOLVER_POSTINGHEU_UCUTFRACT = 4,
    //    SOLVER_MH_HARDFIX=5,
    //    SOLVER_MH_LOCBRANCH=6,
    //}TSPSolvers;
    // DEFAULT SOLVER_BASE

    if (inst->solver == SOLVER_BASE) {
        status = base_cplex(env, lp, inst);
    } else if (inst->solver == SOLVER_BENDER) {
        status = add_subtour_constraints(inst, env, lp);
    } else if (inst->solver == SOLVER_BRANCH_AND_CUT)  {
        status = branch_and_cut(inst, env, lp, 0);
    } else if (inst->solver == SOLVER_PATCHING_HEURISTIC) {
        status = branch_and_cut(
                inst, env, lp,
                CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_RELAXATION);
    } else if (inst->solver == SOLVER_POSTINGHEU_UCUTFRACT){
        status = branch_and_cut(inst, env, lp, CPX_CALLBACKCONTEXT_CANDIDATE);
    } else if (inst->solver == SOLVER_MH_HARDFIX) {
        status = hard_fixing(env, lp, inst);
    } else if (inst->solver == SOLVER_MH_LOCBRANCH) {
        status = local_branching(env, lp, inst);
    } else {
        ERROR_COMMENT("tspcplex.c:solve_problem", "Invalid solver selected");
    }

    if (status!=SUCCESS)
        ERROR_COMMENT("tspcplex.c:solve_problem", "Execution FAILED");
    else if (status == 2)
        ERROR_COMMENT("tspcplex.c:solve_problem", "Time out during execution");
    return status;
}

void TSPopt(Instance *inst) {
    INFO_COMMENT("tspcplex.c:TSPopt", "Solving TSP with CPLEX");
    int error;
    CPXENVptr env = CPXopenCPLEX(&error);
    if (error) ERROR_COMMENT("tspcplex.c:TSPopt", "CPXopenCPLEX() error");
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
    if (error) ERROR_COMMENT("tspcplex.c:TSPopt", "CPXcreateprob() error");

    //------------------------------------------ Building the
    //model----------------------------------------------------------------
    build_model(inst, env, lp);
    INFO_COMMENT("tspcplex.c:TSPopt", "Model built FINISHED");
    //----------------------------------------- Cplex's parameter setting
    //---------------------------------------------------------
    CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
    CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 1234);
    CPXsetdblparam(env, CPX_PARAM_TILIM, 10);  // TODO fix 10 with inst->timelimit
                                               //-----------------------------------------computing the
                                               //solution---------------------------------------------------------------
    int status = solve_problem(env, lp, inst);
    if (status)
        ERROR_COMMENT("tspcplex.c:TSPopt", "Execution FAILED");
    else
        printf("status = %d-------------------------------------------s\n", status);

    //------------------------ free and close cplex model
    //--------------------------------------------------------------
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);
}
