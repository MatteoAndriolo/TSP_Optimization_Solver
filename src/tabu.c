#include "tabu.h"

//-----------------------------------------------------------------------------------------------
// Tabu search
// Initialize the circular buffer
void initBuffer(CircularBuffer *buf, int size)
{
    buf->arr = malloc(size * sizeof(int));
    buf->size = size;
    buf->start = 0;
    buf->end = 0;
    buf->tenure = 0;
}

// Clear the buffer
void clearBuffer(CircularBuffer *buf)
{
    FREE(buf->arr);
    buf->size = 0;
}

// Add a value to the buffer
void addValue(CircularBuffer *buf, int value)
{
    DEBUG_COMMENT("tabu.c:addValue", "adding value %d", value);
    buf->arr[buf->end] = value;
    buf->end = (buf->end + 1) % buf->size;
    buf->tenure++;
    if (buf->end == buf->start)
    {                                              // The buffer is full
        buf->start = (buf->start + 1) % buf->size; // drop the oldest element
    }
}

// Print the buffer values
void printBuffer(CircularBuffer *buf)
{
    int i = buf->start;
    while (i != buf->end)
    {
        printf("%d ", buf->arr[i]);
        i = (i + 1) % buf->size;
    }
    printf("\n");
}

// Initialize an iterator for the buffer
void initIterator(BufferIterator *iter, CircularBuffer *buf)
{
    iter->buf = buf;
    iter->current = buf->start;
    iter->hasStarted = false;
}

bool hasNext(BufferIterator *iter)
{
    if (!iter->buf->tenure || !iter->hasStarted)
    {
        return false;
    }
    int nextIndex = (iter->current + 1) % iter->buf->size;
    return nextIndex != iter->buf->end;
}

int next(BufferIterator *iter)
{
    if (!iter->hasStarted)
    {
        iter->hasStarted = true;
    }
    else
    {
        iter->current = (iter->current + 1) % iter->buf->size;
    }
    return iter->buf->arr[iter->current];
}

// // Check if there are more elements to iterate over
// bool hasNext(BufferIterator *iter)
// {
//     return !iter->buf->isEmpty && (!iter->hasStarted || iter->current != iter->buf->end);
// }

// // Get the next element in the iteration
// int next(BufferIterator *iter)
// {
//     if (!iter->hasStarted)
//     {
//         iter->hasStarted = true;
//     }
//     else
//     {
//         iter->current = (iter->current + 1) % iter->buf->size;
//     }
//     return iter->buf->arr[iter->current];
// }

// Check if the buffer contains a certain value
bool contains(CircularBuffer *buf, int x)
{
    BufferIterator iter;
    initIterator(&iter, buf);

    while (hasNext(&iter))
    {
        if (next(&iter) == x)
        {
            return true;
        }
    }

    return false;
}

/**
 * Decrease tenure size by n
 */
void decreaseSizeTenure(CircularBuffer *buff, int n)
{
    if (buff->tenure == 0)
    {
        FATAL_COMMENT("tabu.c:decreaseSizeTenure", "buffer is empty")
    }
    for (int i = 0; i < n; i++)
    {
        buff->start = (buff->start + 1) % buff->size;
        buff->tenure--;
    }
}

int tabu_stoppingCondition(int currentIteration, int nIterNotFoundImp, int nnodes)
{
    return currentIteration > nnodes * 10 || nIterNotFoundImp > nnodes / 10; // TODO setup stopping condition
}

int tabu_getTabuListSize(int currentIteration, int nnodes)
{
    // return (int)(currentIteration < 10e3 ? currentIteration / 10 : currentIteration / 100);
    return (int)(nnodes < 10e3 ? nnodes / 10 : nnodes / 100);
}

void tabu_search(const double *distance_matrix, int *path, int nnodes, double *tour_length, int minTabuSize, int maxTabuSize, time_t timelimit)
{
    time_t start_time = time(NULL);
    INFO_COMMENT("tabu.c:tabu_search", "Starting tabu search from path with tour length %lf", *tour_length);
    CircularBuffer tabuList;
    bool tenureIncreasing = true;
    int maxTenure = minTabuSize;
    initBuffer(&tabuList, maxTabuSize);

    // incumbment
    int *incumbment_path = malloc(nnodes * sizeof(int));
    memcpy(incumbment_path, path, nnodes * sizeof(int));
    double incumbment_tour_length = *tour_length;

    int currentIteration = 0;

    // TODO clean tabulist after many number of iterations
    // while (currentIteration > nnodes * 4 && nIterNotFoundImp > nnodes && difftime(start_time, time(NULL)) < timelimit)
    DEBUG_COMMENT("tabu.c:tabu_search", "starting tabu search");
    printBuffer(&tabuList);
    while (currentIteration < 600 && difftime(start_time, time(NULL)) < timelimit)
    {
        currentIteration++;
        DEBUG_COMMENT("tabu.c:tabu_search", "iteration: %d", currentIteration);

        // REACH LOCAL MINIMA ------------------------------------------------------------------------------------
        if (currentIteration % 4 == 0)
            two_opt_tabu(distance_matrix, nnodes, path, tour_length, INFINITY, &tabuList);
        DEBUG_COMMENT("tabu.c:tabu_search", "local minima: %lf", *tour_length);

        if (*tour_length < incumbment_tour_length) // UPDATE INCUMBMENT
        {
            DEBUG_COMMENT("tabu.c:tabu_search", "update incumbment: %lf", *tour_length);
            memcpy(incumbment_path, path, nnodes * sizeof(int));
            incumbment_tour_length = *tour_length;
            DEBUG_COMMENT("tabu.c:tabu_search", "update incumbment: %lf", incumbment_tour_length);
        }

        // MAKE RANDOM MOVE --------------------------------------------------------------------------------------
        int a, b;
        int c = 0;
        while (true && c < nnodes)
        {
            a = randomBetween(0, nnodes);
            b = randomBetween(0, nnodes);
            if (a > b)
            {
                int t = a;
                a = b;
                b = t;
            }
            if (a == b || b == a + 1)
                continue;

            if (!(contains(&tabuList, a) || contains(&tabuList, b) || contains(&tabuList, a + 1) || contains(&tabuList, b + 1)))
                break;
            c++;
        }

        two_opt_move(path, a, b, nnodes);

        //*tour_length = get_tour_length(path, nnodes, distance_matrix);

        // MANAGE TABU LIST --------------------------------------------------------------------------------------

        if (tenureIncreasing && maxTenure < maxTabuSize)
        {
            maxTenure++;
        }
        else if (!tenureIncreasing && maxTenure > minTabuSize)
        {
            maxTenure--;
        }

        addValue(&tabuList, a);
        addValue(&tabuList, b);
        if (!tenureIncreasing)
        {
            if (maxTenure - 4 < minTabuSize)
            {
                decreaseSizeTenure(&tabuList, maxTenure - minTabuSize);
                tenureIncreasing = true;
            }
            else
                decreaseSizeTenure(&tabuList, 4);
        }

        if (maxTenure >= maxTabuSize)
            tenureIncreasing = false;
        if (maxTenure <= minTabuSize)
            tenureIncreasing = true;

        // if (currentIteration % 40 == 0)
        // {
        //     DEBUG_COMMENT("tabu.c:tabu_search", "clear buffer");
        //     clearBuffer(&tabuList);
        //     initBuffer(&tabuList, maxTabuSize);
        //     two_opt(distance_matrix, nnodes, path, tour_length, INFINITY);
        // }
        // DEBUG -------------------------------------------------------------------------------------------------
        printf("iter %d tenure %d, %s\n", currentIteration, tabuList.tenure, tenureIncreasing ? "increasing" : "decreasing");
        ffflush();
        DEBUG_COMMENT("tabu.c:tabu_search", "tabu list size: %d", tabuList.size);
        INFO_COMMENT("tabu.c:tabu_search", "end iteration: %d\t lenght %lf\t encumbment %lf", currentIteration, *tour_length, incumbment_tour_length);
    }
    DEBUG_COMMENT("tabu.c:tabu_search", "exit main loop");
    // CLOSE UP -  return best encumbment
    memcpy(path, incumbment_path, nnodes * sizeof(int));
    two_opt(distance_matrix, nnodes, path, tour_length, INFINITY);
    clearBuffer(&tabuList);
    FREE(incumbment_path);
    INFO_COMMENT("tabu.c:tabu_search", "end tabu search: %lf", *tour_length);
}

void test_buffer()
{
    CircularBuffer buf;
    initBuffer(&buf, 10);
    printf("init:\n");
    printBuffer(&buf);

    printf("iterating: ");
    BufferIterator iter;
    initIterator(&iter, &buf);
    while (hasNext(&iter))
    {
        printf("%d ", next(&iter));
    }
    printf("\n\n");

    for (int i = 0; i < 30; i++)
    {
        addValue(&buf, i);
        printBuffer(&buf);
        printf("iterating: ");
        BufferIterator iter;
        initIterator(&iter, &buf);
        while (hasNext(&iter))
        {
            printf("%d ", next(&iter));
        }
        printf("\n\n");
    }

    clearBuffer(&buf);
}