#ifndef REFINEMENT_H
#define REFINEMENT_H
#include <time.h>

#include "../include/tabu.h"
#include "../include/vrp.h"
#include "../include/utils.h"

/**
 * Applies the 2-opt algorithm to a given distance matrix and node list (in
 * place).
 *
 * @param inst A pointer to the instance.
 * @param iterations The number of iterations to perform. (INFINITY: until no
 * improvement is found)
 * @return void
 */
int two_opt(Instance *inst, double iterations);

/**
 * Applies the 2-opt algorithm to a given distance matrix and node list (in
 * place), using a tabu list.
 *
 * @param inst A pointer to the instance.
 * @param iterations The number of iterations to perform. (INFINITY: until no improvement is found)
 * @param tabuList A pointer to the tabu list.
 */
int two_opt_tabu(Instance *inst, double iterations, TabuList *tabuList);
#endif
