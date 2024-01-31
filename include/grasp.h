#ifndef GRASP_H
#define GRASP_H
#include <stdbool.h>

typedef struct {
  int solution;
  double value;
} Solution;

typedef struct {
  Solution *solutions;
  int count;
  double *probabilities;
  int size;
} GRASP_Framework;

void GRASP_init(GRASP_Framework *grasp, double probabilities[], int size);
void GRASP_addSolution(GRASP_Framework *grasp, int solution, double value);
int GRASP_getSolution(GRASP_Framework *grasp);
void GRASP_free(GRASP_Framework *grasp);
void GRASP_resetSolutions(GRASP_Framework *grasp);
bool GRASP_isInGrasp(GRASP_Framework *grasp, int solution);

#endif  // GRASP_H
