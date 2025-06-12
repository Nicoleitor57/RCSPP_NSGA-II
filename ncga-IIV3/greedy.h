#ifndef GREEDY_H
#define GREEDY_H

#include "model.h"

// Genera una cantidad `num_solutions` de soluciones válidas
void generate_initial_solutions(const Instance *inst, Solution *solutions, int num_solutions);
int is_in_path(int node, const Solution *sol, const Instance *inst);
void shuffle(int *array, int n);

#endif
