#ifndef GREEDY_H
#define GREEDY_H

#include "model.h"

// Genera una cantidad `num_solutions` de soluciones válidas
void generate_initial_solutions(const Instance *inst, Solution *solutions, int num_solutions);
int is_in_path(int node, const Solution *sol, const Instance *inst);
void shuffle(int *array, int n);

void generate_random_valid_solution(Solution *sol, const Instance *inst);
void generate_greedy_solution(Solution *sol, const Instance *inst);
void populate_solution_from_path(Solution *sol, const int* path_arcs, int path_len, int num_total_arcs);
//bool is_in_path(int node_id, const Solution *sol, const Instance *inst);

#endif
