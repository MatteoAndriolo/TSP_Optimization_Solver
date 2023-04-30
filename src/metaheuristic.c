#include "metaheuristic.h"

double randomBetween_d(double lowerBound, double upperBound)
{
    double randomBetween = (double)rand() / RAND_MAX;
    randomBetween = randomBetween * (upperBound - lowerBound) + lowerBound;
    return randomBetween;
}

void simulate_anealling(const double *distance_matrix, int *path, int nnodes, double *tour_length, double T, int duration)
{
    INFO_COMMENT("metaheuristic.c:simulate_anealling", "Starting simulated annealing metaheuristic");
    double curren_tour_lenght = *tour_length;
    double best_tour_length = *tour_length;
    double next_tour_length = *tour_length;
    int *current_path = malloc(nnodes * sizeof(int));
    int *next_tour = malloc(nnodes * sizeof(int));
    int *best_tour = malloc(nnodes * sizeof(int));
    memcpy(best_tour, path, nnodes * sizeof(int));
    memcpy(current_path, path, nnodes * sizeof(int));
    memcpy(next_tour, path, nnodes * sizeof(int));
    time_t start_time = time(NULL);
    time_t end_time = start_time + duration;
    double delta_E = 0;

    while (T > 0 && difftime(end_time, time(NULL)) > 0)
    {
        T -= 1; // lower slowly the temperature
        // T high a lot of kick function done 
        kick_function(distance_matrix, path, nnodes, tour_length, (int)T);
        memcpy(next_tour, path, nnodes * sizeof(int));
        next_tour_length = *tour_length;
        delta_E = next_tour_length - curren_tour_lenght;
        if (delta_E > 0)
        {
            two_opt(distance_matrix, nnodes,next_tour, &next_tour_length, INFINITY);
            memcpy(current_path, next_tour, nnodes * sizeof(int));
            curren_tour_lenght = next_tour_length;
            CRITICAL_COMMENT("metaheuristic.c:simulate_anealling", " BETTER SOLUTION FOUND - %lf", delta_E);
            if (curren_tour_lenght < best_tour_length)
            {
                best_tour_length = curren_tour_lenght;
                memcpy(best_tour, current_path, nnodes * sizeof(int));
                CRITICAL_COMMENT("metaheuristic.c:simulate_anealling", " BETTER SOLUTION FOUND - %lf", delta_E);
            }
        }
        else
        {
            if (randomBetween_d(0, 1) < exp(delta_E / T))
            {
                CRITICAL_COMMENT("metaheuristic.c:simulate_anealling", " ALLOWED BAD MOVE - %lf", delta_E);
                memcpy(current_path, next_tour, nnodes * sizeof(int));
                curren_tour_lenght = next_tour_length;
            }
        }
    }
    memcpy(path, best_tour, nnodes * sizeof(int));
    *tour_length = best_tour_length;
    INFO_COMMENT("metaheuristic.c:simulate_anealling", "Finished simulated annealing metaheuristic, best path found= %lf", *tour_length);

}