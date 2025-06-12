#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "global.h"
#include "rand.h"

// Prototipos de funciones definidas en este fichero o en otros necesarios
void path_based_crossover(individual *p1, individual *p2, individual *c1, individual *c2, Instance *inst);
void rebuild_genes_from_path(individual *ind, const int* new_arc_path, int path_len, int num_total_arcs);
bool randomized_bfs_patch(Instance *inst, int from_node, int to_node, int* patch_nodes, int* patch_len, int arc_to_ignore);
int find_arc_index(Instance *inst, int from, int to);
void shuffle_array(int *array, int n);

// =============================================================================
// FUNCIÓN PRINCIPAL DE CRUCE (Llamada desde nsga2.c)
// =============================================================================
void crossover(individual *parent1, individual *parent2, individual *child1, individual *child2, Instance *instance) {
    path_based_crossover(parent1, parent2, child1, child2, instance);
}

// --- OPERADOR DE CRUCE PRINCIPAL "ANTI-COLAPSO" ---
void path_based_crossover(individual *p1, individual *p2, individual *c1, individual *c2, Instance *inst) {
    const int num_nodes = inst->sink + 1; // CORREGIDO: Usar sink + 1

    int p1_len = 0; while (p1_len < nbits[0] && p1->gene[0][p1_len] != -1) p1_len++;
    int p2_len = 0; while (p2_len < nbits[0] && p2->gene[0][p2_len] != -1) p2_len++;

    if (p1_len < 1 || p2_len < 1) {
        memcpy(c1, p1, sizeof(individual)); memcpy(c2, p2, sizeof(individual)); return;
    }

    int *p1_nodes = (int*)malloc(num_nodes * sizeof(int));
    int *p2_nodes = (int*)malloc(num_nodes * sizeof(int));
    bool *in_p1 = (bool*)malloc(num_nodes * sizeof(bool));
    int *common_nodes = (int*)malloc(num_nodes * sizeof(int));

    get_node_path_from_arcs(inst, p1->gene[0], p1_len, p1_nodes); // CORREGIDO: Llamada con 4 parámetros
    get_node_path_from_arcs(inst, p2->gene[0], p2_len, p2_nodes); // CORREGIDO: Llamada con 4 parámetros

    memset(in_p1, false, num_nodes * sizeof(bool));
    for(int i = 1; i < p1_len; i++) { in_p1[p1_nodes[i]] = true; }

    int common_count = 0;
    for (int i = 1; i < p2_len; i++) {
        if (in_p1[p2_nodes[i]]) { common_nodes[common_count++] = p2_nodes[i]; }
    }

    if (common_count > 0) {
        // CASO 1: HAY NODOS COMUNES
        int crossover_node_id = common_nodes[rnd(0, common_count - 1)];
        int p1_cross_idx = -1, p2_cross_idx = -1;
        for(int i = 0; i < p1_len; i++) if (inst->arcs[p1->gene[0][i]].to.id == crossover_node_id) { p1_cross_idx = i; break; }
        for(int i = 0; i < p2_len; i++) if (inst->arcs[p2->gene[0][i]].to.id == crossover_node_id) { p2_cross_idx = i; break; }

        if (p1_cross_idx != -1 && p2_cross_idx != -1) {
            int c1_path_arcs[nbits[0]], c2_path_arcs[nbits[0]];
            int c1_len = 0, c2_len = 0;

            for(int i = 0; i <= p1_cross_idx; i++) c1_path_arcs[c1_len++] = p1->gene[0][i];
            for(int i = p2_cross_idx + 1; i < p2_len; i++) c1_path_arcs[c1_len++] = p2->gene[0][i];
            rebuild_genes_from_path(c1, c1_path_arcs, c1_len, inst->num_arcs);

            for(int i = 0; i <= p2_cross_idx; i++) c2_path_arcs[c2_len++] = p2->gene[0][i];
            for(int i = p1_cross_idx + 1; i < p1_len; i++) c2_path_arcs[c2_len++] = p1->gene[0][i];
            rebuild_genes_from_path(c2, c2_path_arcs, c2_len, inst->num_arcs);
        } else {
            memcpy(c1, p1, sizeof(individual)); memcpy(c2, p2, sizeof(individual));
        }
    } else {
        // CASO 2: NO HAY NODOS COMUNES
        memcpy(c1, p1, sizeof(individual)); memcpy(c2, p2, sizeof(individual));
    }
    free(p1_nodes); free(p2_nodes); free(in_p1); free(common_nodes);
}

// --- DEFINICIONES DE FUNCIONES AUXILIARES ---

// CORREGIDO: La firma ahora coincide con global.h (4 parámetros)
void get_node_path_from_arcs(Instance *inst, const int* arc_path, int path_len, int* node_path) {
    if (path_len == 0) return;
    node_path[0] = inst->arcs[arc_path[0]].from.id;
    for (int i = 0; i < path_len; ++i) {
        node_path[i + 1] = inst->arcs[arc_path[i]].to.id;
    }
}

void rebuild_genes_from_path(individual *ind, const int* new_arc_path, int path_len, int num_total_arcs) {
    for (int i = 0; i < path_len; i++) ind->gene[0][i] = new_arc_path[i];
    for (int i = path_len; i < nbits[0]; i++) ind->gene[0][i] = -1;
    memset(ind->gene[1], 0, sizeof(int) * num_total_arcs);
    for (int i = 0; i < path_len; i++) ind->gene[1][new_arc_path[i]] = 1;
}

int find_arc_index(Instance *inst, int from, int to) {
    for (int i = 0; i < inst->out_degrees[from]; i++) {
        int arc_idx = inst->outgoing_arcs[from][i];
        if (inst->arcs[arc_idx].to.id == to) return arc_idx;
    }
    return -1;
}

void shuffle_array(int *array, int n) {
    if (n > 1) {
        for (int i = 0; i < n - 1; i++) {
          int j = i + rnd(0, n - 1 - i);
          int temp = array[j];
          array[j] = array[i];
          array[i] = temp;
        }
    }
}

bool randomized_bfs_patch(Instance *inst, int from_node, int to_node, int* patch_nodes, int* patch_len, int arc_to_ignore) {
    const int num_nodes = inst->sink + 1; // CORREGIDO
    if (from_node >= num_nodes || to_node >= num_nodes) return false;

    int* queue = (int*)malloc(num_nodes * sizeof(int));
    int* parent = (int*)malloc(num_nodes * sizeof(int));
    bool* visited = (bool*)malloc(num_nodes * sizeof(bool));
    memset(visited, false, num_nodes * sizeof(bool));

    int front = 0, back = 0;
    queue[back++] = from_node;
    visited[from_node] = true;
    parent[from_node] = -1;
    bool found = false;

    while (front < back) {
        int u = queue[front++];
        if (u == to_node) { found = true; break; }
        
        int* neighbors_arcs = (int*)malloc(inst->out_degrees[u] * sizeof(int));
        for(int i=0; i < inst->out_degrees[u]; ++i) neighbors_arcs[i] = inst->outgoing_arcs[u][i];
        shuffle_array(neighbors_arcs, inst->out_degrees[u]);

        for (int i = 0; i < inst->out_degrees[u]; i++) {
            int current_arc_idx = neighbors_arcs[i];
            if (current_arc_idx == arc_to_ignore) continue;
            int v = inst->arcs[current_arc_idx].to.id;
            if (!visited[v]) {
                visited[v] = true;
                parent[v] = u;
                queue[back++] = v;
            }
        }
        free(neighbors_arcs);
    }

    if (found) {
        int temp_path[num_nodes];
        int len = 0;
        int curr = to_node;
        while (curr != -1) { temp_path[len++] = curr; curr = parent[curr]; }
        for(int i = 0; i < len; i++) patch_nodes[i] = temp_path[len - 1 - i];
        *patch_len = len;
    }
    free(queue); free(parent); free(visited);
    return found;
}