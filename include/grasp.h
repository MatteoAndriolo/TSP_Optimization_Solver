#ifndef GRASP_H
#define GRASP_H

typedef struct {
    int solution;
    double value;
} Solution;

typedef struct {
    Solution* solutions;
    int count;
    double* probabilities;
    int size;
} GRASP_Framework;

void init_grasp(GRASP_Framework* grasp, double probabilities[], int size);
void add_solution(GRASP_Framework* grasp, int solution, double value);
int get_solution(GRASP_Framework* grasp);
void free_grasp(GRASP_Framework* grasp);
void reset_solutions(GRASP_Framework* grasp);

#endif // GRASP_H

