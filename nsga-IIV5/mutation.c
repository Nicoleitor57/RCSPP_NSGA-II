/* Mutation routines */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>
#include <stdbool.h>
#include <string.h>


# include "global.h"
# include "rand.h"

#define MAX_PATCH_LENGTH 4  // Cuántos arcos máximo se pueden usar para reemplazar 1
#define MAX_ATTEMPTS 10     // Intentos antes de abortar la mutación
#define MAX_NODES 10000
#define MAX_GENES 50000

/* Function to perform mutation in a population */
void mutation_pop (population *pop, Instance *inst)
{
    int i;
    for (i=0; i<popsize; i++)
    {
        mutation_ind(&(pop->ind[i]), inst);
    }
    return;
}

bool try_mutation(individual *ind, Instance *inst) {
    // Obtener longitud de la ruta desde gene[0]
    int route_len = 0;
    while (route_len < nbits[0] && ind->gene[0][route_len] != -1)
        route_len++;

    if (route_len < 3) return false;

    for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
        int i = rnd(1, route_len - 2);
        int from = inst->arcs[ind->gene[0][i]].from.id;
        int to = inst->arcs[ind->gene[0][i + 1]].to.id;

        int patch[MAX_PATCH_LENGTH];
        int patch_len = -1;

        bool found = bfs_patch(inst, from, to, patch, &patch_len);
        if (!found || patch_len < 1) continue;

        // Reconstruir el nuevo camino
        int new_path[MAX_GENES];
        int k = 0;

        // Copiar la parte antes del punto de mutación
        for (int j = 0; j < i; j++) {
            new_path[k++] = ind->gene[0][j];
        }

        // Insertar el parche
        for (int j = 0; j < patch_len - 1; j++) {
            int u = patch[j];
            int v = patch[j + 1];
            int arc = find_arc_index(inst, u, v);
            if (arc == -1) {
                printf("ERROR: Arco no encontrado entre %d y %d\n", u, v);
                return false;
            }
            new_path[k++] = arc;
        }

        // Copiar el resto de la ruta
        for (int j = i + 1; j < route_len; j++) {
            new_path[k++] = ind->gene[0][j];
        }

        // Reconstruir gene[0] y gene[1]
        for (int j = 0; j < nbits[0]; j++) ind->gene[0][j] = -1;         // limpiar gene[0]
        for (int j = 0; j < nbits[1]; j++) ind->gene[1][j] = 0;          // limpiar gene[1]

        for (int j = 0; j < k; j++) {
            ind->gene[0][j] = new_path[j];
            ind->gene[1][new_path[j]] = 1;
        }

        return true;
    }

    return false;
}

int find_arc_index(Instance *inst, int from, int to) {
    for (int i = 0; i < inst->out_degrees[from]; i++) {
        int arc_idx = inst->outgoing_arcs[from][i];
        if (inst->arcs[arc_idx].to.id == to) {
            return arc_idx;
        }
    }
    return -1; // No encontrado
}

bool bfs_patch(Instance *inst, int from, int to, int *patch, int *patch_len) {
    int queue[MAX_NODES];
    int parent[MAX_NODES];
    int depth[MAX_NODES];
    bool visited[MAX_NODES] = {false};

    int front = 0, back = 0;
    queue[back++] = from;
    visited[from] = true;
    parent[from] = -1;
    depth[from] = 0;

    while (front < back) {
        int u = queue[front++];

        if (depth[u] >= MAX_PATCH_LENGTH) continue;

        for (int i = 0; i < inst->out_degrees[u]; i++) {
            int arc_idx = inst->outgoing_arcs[u][i];
            int v = inst->arcs[arc_idx].to.id;

            if (!visited[v]) {
                visited[v] = true;
                parent[v] = u;
                depth[v] = depth[u] + 1;

                if (v == to) {
                    // reconstruir ruta desde 'to' a 'from'
                    int path[MAX_PATCH_LENGTH + 1];
                    int len = 0, curr = to;
                    while (curr != -1) {
                        path[len++] = curr;
                        curr = parent[curr];
                    }

                    // invertir para obtener from → to
                    for (int j = 0; j < len; j++) {
                        patch[j] = path[len - j - 1];
                    }
                    *patch_len = len;
                    return true;
                }

                queue[back++] = v;
            }
        }
    }

    return false; // No se encontró camino
}

void mutation_ind(individual *ind, Instance *inst) {
    if (randomperc() < pmut_bin) {
        try_mutation(ind, inst);
    }
}


// void mutation_ind (individual *ind, Instance *inst)
// {
//     if (randomperc() < pmut_real)
//     {
//         int route_len = ind->sol->length;
//         if (route_len <= 2) return; // No mutation for routes of length 2 or less

//         int i = rnd(1, route_len - 2);
//         int from_node = inst->arcs[ind->sol->arc_used_index[i]].from.id; //completamente criminal
//         int sink_node = inst->arcs[ind->sol->arc_used_index[i+1]].to.id;

//         int current = from_node;
//         //int new_gene[5];
//         int new_index = 0;

//         while (current != sink_node && new_index < 5){

//         }
        
//     }
// }

/* Function to perform mutation of an individual */
// void mutation_ind (individual *ind)
// {
//     if (nreal!=0)
//     {
//         real_mutate_ind(ind);
//     }
//     if (nbin!=0)
//     {
//         bin_mutate_ind(ind);
//     }
//     return;
// }

// /* Routine for binary mutation of an individual */
// void bin_mutate_ind (individual *ind)
// {
//     int j, k;
//     double prob;
//     for (j=0; j<nbin; j++)
//     {
//         for (k=0; k<nbits[j]; k++)
//         {
//             prob = randomperc();
//             if (prob <=pmut_bin)
//             {
//                 if (ind->gene[j][k] == 0)
//                 {
//                     ind->gene[j][k] = 1;
//                 }
//                 else
//                 {
//                     ind->gene[j][k] = 0;
//                 }
//                 nbinmut+=1;
//             }
//         }
//     }
//     return;
// }

// /* Routine for real polynomial mutation of an individual */
// void real_mutate_ind (individual *ind)
// {
//     int j;
//     double rnd, delta1, delta2, mut_pow, deltaq;
//     double y, yl, yu, val, xy;
//     for (j=0; j<nreal; j++)
//     {
//         if (randomperc() <= pmut_real)
//         {
//             y = ind->xreal[j];
//             yl = min_realvar[j];
//             yu = max_realvar[j];
//             delta1 = (y-yl)/(yu-yl);
//             delta2 = (yu-y)/(yu-yl);
//             rnd = randomperc();
//             mut_pow = 1.0/(eta_m+1.0);
//             if (rnd <= 0.5)
//             {
//                 xy = 1.0-delta1;
//                 val = 2.0*rnd+(1.0-2.0*rnd)*(pow(xy,(eta_m+1.0)));
//                 deltaq =  pow(val,mut_pow) - 1.0;
//             }
//             else
//             {
//                 xy = 1.0-delta2;
//                 val = 2.0*(1.0-rnd)+2.0*(rnd-0.5)*(pow(xy,(eta_m+1.0)));
//                 deltaq = 1.0 - (pow(val,mut_pow));
//             }
//             y = y + deltaq*(yu-yl);
//             if (y<yl)
//                 y = yl;
//             if (y>yu)
//                 y = yu;
//             ind->xreal[j] = y;
//             nrealmut+=1;
//         }
//     }
//     return;
// }
