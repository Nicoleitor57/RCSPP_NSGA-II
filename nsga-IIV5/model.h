#ifndef MODEL_H
#define MODEL_H

#include "GraphReader.h"
#include <stdbool.h>

#define NUM_OBJECTIVES 2




typedef struct {
    int *arc_used;  // arreglo de 0 o 1 por arco
    int length;  // número de arcos
    float *arrival_time;   // tiempo de llegada a cada nodo
    int *arc_used_index;
    int *violated_nodes; // nodos que violan la ventana de tiempo
    float objectives[NUM_OBJECTIVES];
} Solution;

// Función para evaluar solo función objetivo
void evaluate_objective(const Instance *instance, Solution *sol, double *dist, double *time);
int check_flow_balance(const Instance *instance, const Solution *sol);
int check_no_subtours(const Instance *instance, const Solution *sol);
int check_resource_limit(const Instance *instance, const Solution *sol);
int check_no_forbidden_nodes(const Instance *instance, const Solution *sol);
int check_required_nodes(const Instance *instance, const Solution *sol);
int validate_time_windows(const Solution *sol, const Instance *inst, float *arrival_time);
bool already_recorded(int node, int *violated_nodes, int count);

#endif // MODEL_H
