#include "../include/heuristics.h"

void vnp_k(Instance *inst, int k)
{
    INFO_COMMENT("heuristics.c:vnp_k", "starting the huristics loop for vnp_k");
    //TODO fix while loop
    int c=0;
    while (++c<1000)
    {
        two_opt(inst, INFINITY);
        kick_function(inst, k);
        pathCheckpoint(inst);
    }
    inst->path = inst->best_path;
    inst->tour_length = inst->best_tourlength;
    INFO_COMMENT("heuristics.c:vnp_k", "finished the huristics loop for vnp_k, best inst->path found= %lf", inst->tour_length);
}

void kick_function(Instance *inst, int k)
{
    int found = 0;
    int index_0, index_1, index_2, index_3, index_4;
    while (found != 4)
    {
        found = 0;
        index_0 = 1;
        found++;
        index_1 = randomBetween(index_0 + 1, inst->nnodes);
        if (inst->nnodes - index_1 > 6)
        {
            index_2 = randomBetween(index_1 + 2, inst->nnodes);
            found++;
        }
        if (inst->nnodes - index_2 > 4)
        {
            index_3 = randomBetween(index_2 + 2, inst->nnodes);
            found++;
        }
        if (inst->nnodes - index_3 > 2)
        {
            index_4 = randomBetween(index_3 + 2, inst->nnodes);
            found++;
        }
        if (found == 4)
        {
            index_0--;
            index_1--;
            index_2--;
            index_3--;
            index_4--;
            // DEBUG_COMMENT("heuristics.c:kick_function", "found 5 indexes{%d, %d, %d, %d, %d}", index_0, index_1, index_2, index_3, index_4);
        }
    }
    //---------------------------------------------------------------------------------------
    int *final_path = malloc(inst->nnodes * sizeof(int));
    int count = 0;
    for (int i = 0; i < index_1; i++)
        final_path[count++] = inst->path[i];
    for (int i = index_4; i < inst->nnodes; i++)
        final_path[count++] = inst->path[i];
    for (int i = index_4 - 1; i >= index_3; i--)
        final_path[count++] = inst->path[i];
    for (int i = index_2; i < index_3; i++)
        final_path[count++] = inst->path[i];
    for (int i = index_2 - 1; i >= index_1; i--)
        final_path[count++] = inst->path[i];
    free(inst->path);
    inst->path = final_path;
}
