#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "global.h"
#include "rand.h"

// Prototipos de funciones auxiliares necesarias (definidas en otros ficheros)
// Esto soluciona las dependencias de compilación.
void rebuild_genes_from_path(individual *ind, const int* new_arc_path, int path_len, int num_total_arcs);
bool randomized_bfs_patch(Instance *inst, int from_node, int to_node, int* patch_nodes, int* patch_len, int arc_to_ignore);
int find_arc_index(Instance *inst, int from, int to);

// Prototipos de funciones de mutación definidas en este fichero
bool diversified_mutation(individual *ind, Instance *inst);
bool inversion_mutation(individual *ind, Instance *inst);

// =============================================================================
// FUNCIÓN PRINCIPAL DE MUTACIÓN (Llamada desde nsga2.c)
// =============================================================================
void mutation_pop(population *pop, Instance *inst) {
    for (int i = 0; i < popsize; i++) {
        mutation_ind(&(pop->ind[i]), inst);
    }
}

// --- "SUPER-MUTADOR" ---
void mutation_ind(individual *ind, Instance *inst) {
    if (randomperc() < pmut_bin) {
        if (randomperc() <= 0.5) {
            inversion_mutation(ind, inst);
        } else {
            diversified_mutation(ind, inst);
        }
    }
}

// --- MUTACIÓN 1: POR REEMPLAZO DE ARCO CON DESVÍO ---
bool diversified_mutation(individual *ind, Instance *inst) {
    int route_len = 0;
    while (route_len < nbits[0] && ind->gene[0][route_len] != -1) route_len++;
    if (route_len < 3) return false;

    for (int attempt = 0; attempt < 10; attempt++) {
        int i = rnd(1, route_len - 2);
        int arc_to_replace_idx = ind->gene[0][i];
        int from_node = inst->arcs[arc_to_replace_idx].from.id;
        int to_node = inst->arcs[arc_to_replace_idx].to.id;

        if (from_node == to_node) continue;

        const int num_nodes = inst->sink + 1; // CORREGIDO
        int *patch_nodes = (int*)malloc(num_nodes * sizeof(int));
        int patch_len = 0;
        bool found = randomized_bfs_patch(inst, from_node, to_node, patch_nodes, &patch_len, arc_to_replace_idx);

        if (found && patch_len > 2) {
            int new_path_arcs[nbits[0]];
            int k = 0;
            for (int l = 0; l < i; l++) new_path_arcs[k++] = ind->gene[0][l];
            for (int l = 0; l < patch_len - 1; l++) {
                int arc_idx = find_arc_index(inst, patch_nodes[l], patch_nodes[l+1]);
                if (arc_idx != -1) new_path_arcs[k++] = arc_idx;
            }
            for (int l = i + 1; l < route_len; l++) new_path_arcs[k++] = ind->gene[0][l];
            rebuild_genes_from_path(ind, new_path_arcs, k, inst->num_arcs);
            free(patch_nodes);
            return true;
        }
        free(patch_nodes);
    }
    return false;
}

// --- MUTACIÓN 2: POR INVERSIÓN DE SECUENCIA ---
bool inversion_mutation(individual *ind, Instance *inst) {
    int route_len = 0;
    while (route_len < nbits[0] && ind->gene[0][route_len] != -1) route_len++;
    if (route_len < 3) return false;

    int cut1 = rnd(0, route_len - 1);
    int cut2 = rnd(0, route_len - 1);

    if (cut1 == cut2) return false;
    if (cut1 > cut2) { int temp = cut1; cut1 = cut2; cut2 = temp; }

    while (cut1 < cut2) {
        int temp = ind->gene[0][cut1];
        ind->gene[0][cut1] = ind->gene[0][cut2];
        ind->gene[0][cut2] = temp;
        cut1++;
        cut2--;
    }
    return true;
}