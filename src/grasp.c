#include "../include/grasp.h"
#include "../include/logger.h"
#include "../include/errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

// Helper function to compare two solutions based on their values. Used for sorting.
int compare(const void* a, const void* b) {
    double difference = ((Solution*)a)->value - ((Solution*)b)->value;
    if (difference < 0.0) return -1;
    else if (difference > 0.0) return 1;
    else return 0;
}

void init_grasp(GRASP_Framework* grasp, double probabilities[], int size) {
    grasp->count = 0;
    grasp->size = size;
    grasp->solutions = (Solution*)malloc(size * sizeof(Solution));
    grasp->probabilities = (double*)malloc(size * sizeof(double));

    if (grasp->solutions == NULL || grasp->probabilities == NULL) {
        ERROR_COMMENT("init_grasp", "malloc failed");
        exit(FAILURE);
    }

    for (int i = 0; i < size; i++) {
        grasp->solutions[i].solution = -1;
        grasp->solutions[i].value = INFINITY;
    }

    for (int i = 0; i < size; i++) {
        grasp->probabilities[i] = probabilities[i];
    }
    INFO_COMMENT("init_grasp", "GRASP initialized");
    INFO_COMMENT("init_grasp", "GRASP size: %d", size);
    INFO_COMMENT("init_grasp", "GRASP probabilities:");
    for (int i = 0; i < size; i++) {
        INFO_COMMENT("init_grasp", "GRASP probabilities[%d]: %f", i, probabilities[i]);
    }
}

void add_solution(GRASP_Framework* grasp, int solution, double value) {
    if (grasp->count < grasp->size) {
        grasp->solutions[grasp->count].solution = solution;
        grasp->solutions[grasp->count].value = value;
        grasp->count++;
    } else {
        // Replace the worst solution if the new solution is better
        if (value < grasp->solutions[grasp->size - 1].value) {
            grasp->solutions[grasp->size - 1].solution = solution;
            grasp->solutions[grasp->size - 1].value = value;
        }
    }

    // Sort the solutions in ascending order of value
    qsort(grasp->solutions, grasp->count, sizeof(Solution), compare);
}

int get_solution(GRASP_Framework* grasp) {
    double random_value = (double)rand() / RAND_MAX;

    for (int i = 0; i < grasp->count; i++) {
        if (random_value <= grasp->probabilities[i]) {
            DEBUG_COMMENT("get_solution", "returned %d-th, prob %lf", i, random_value);
            return grasp->solutions[i].solution;
        }
    }

    return grasp->solutions[grasp->count - 1].solution;
}

void reset_solutions(GRASP_Framework* grasp){
    for (int i = 0; i < grasp->size; i++) {
        grasp->solutions[i].solution = -1;
        grasp->solutions[i].value = INFINITY;
    }
    grasp->count = 0;
}

void free_grasp(GRASP_Framework* grasp) {
    free(grasp->solutions);
    free(grasp->probabilities);
    grasp->solutions = NULL;
    grasp->probabilities = NULL;
    grasp->count = 0;
    grasp->size = 0;
}

