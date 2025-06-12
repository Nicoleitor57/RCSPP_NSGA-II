#define DEBUG_PRINTS 0
#if DEBUG_PRINTS
#define DPRINT(...) printf(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

#include "model.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <float.h>
#include <stdbool.h>
#include <assert.h>

void evaluate_objective(const Instance *instance, Solution *sol, double *dist, double *time_) {
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned)time(NULL));
        seeded = 1;
    }
    
    float total_dist = 0.0f;
    float total_time = 0.0f;
    *dist = 0.0f;
    *time_ = 0.0f;

    //impririr los arcos usados
    // for (int i = 0; i < instance->num_arcs; i++) {
    //     if (sol->arc_used[i]) {
    //         printf("arco %d: %d -> %d, usado: %d\n", i, instance->arcs[i].from.id, instance->arcs[i].to.id, sol->arc_used[i]);
    //     }
    // }

    for (int i = 0; i < sol->length; i++) {
        if (sol->arc_used[i]) {

            float rand_val = (float)rand() / (float)RAND_MAX;
            float risk_sum = (float)instance->arcs[i].from.risk + (float)instance->arcs[i].to.risk;
            float risk_multiplier = 1.0f;
            if (rand_val < risk_sum) {
                risk_multiplier = 1.5f;
            }

            DPRINT("arco %d: %d -> %d, riesgo: %f\n", i, instance->arcs[i].from.id, instance->arcs[i].to.id, risk_sum);
            DPRINT("distancia: %d, tiempo: %d\n", instance->arcs[i].dist, instance->arcs[i].time);
            total_dist += (float)instance->arcs[i].dist + (float)instance->arcs[i].epsilon;
            total_time += ((float)instance->arcs[i].time + (float)instance->arcs[i].epsilon) * risk_multiplier;
            DPRINT("multiplicador de riesgo: %f\n", risk_multiplier);
            DPRINT("distancia: %f, tiempo: %f\n", total_dist, total_time);
        }
    }

    sol->objectives[0] = total_dist;
    sol->objectives[1] = total_time;
    *dist = total_dist;
    *time_ = total_time;

    //printf("distancia total: %f, tiempo total: %f\n", total_dist, total_time);

}

int check_flow_balance(const Instance *instance, const Solution *sol) {
    int num_nodes = instance->sink + 1;  // IDs van de 0 a sink
    // Array para llevar cuenta del flujo en cada nodo
    int *flow = calloc(num_nodes, sizeof(int));
    if (!flow) {
        fprintf(stderr, "Error de memoria en check_flow_balance.\n");
        return 0;
    }

    assert(instance->num_arcs > 0);
    assert(sol->arc_used != NULL);

    // Recorremos los arcos y verificamos los usados en la solución
    for (int k = 0; k < instance->num_arcs; k++) {
        // printf("Verificando arco %d: %d -> %d, usado: %d\n",
        //        k, instance->arcs[k].from.id, instance->arcs[k].to.id, sol->arc_used[k]);
        if (sol->arc_used[k]) {
            int from_id = instance->arcs[k].from.id;
            int to_id = instance->arcs[k].to.id;

            assert(from_id >= 0 && from_id < num_nodes);
            assert(to_id >= 0 && to_id < num_nodes);

            if (from_id < 0 || from_id >= num_nodes || to_id < 0 || to_id >= num_nodes) {
                printf("Error: índice fuera de rango en arco %d: from %d -> to %d\n", k, from_id, to_id);
                exit(1);
            }


            flow[from_id]++;
            flow[to_id]--;
        }
    }

    // Verificación del flujo
    for (int i = 0; i < num_nodes; i++) {
        if (i == instance->source) {
            if (flow[i] != 1) {
                DPRINT("Error de flujo en nodo fuente n%d: esperado +1, obtenido %d\n", i + 1, flow[i]);
                free(flow);
                return 0;
            }
        } else if (i == instance->sink) {
            if (flow[i] != -1) {
                DPRINT("Error de flujo en nodo sumidero n%d: esperado -1, obtenido %d\n", i + 1, flow[i]);
                free(flow);
                return 0;
            }
        } else {
            if (flow[i] != 0) {
                DPRINT("Error de flujo en nodo intermedio n%d: esperado 0, obtenido %d\n", i + 1, flow[i]);
                free(flow);
                return 0;
            }
        }
    }

    free(flow);
    return 1;  // Restricción satisfecha
}

int check_no_subtours(const Instance *instance, const Solution *sol) {
    int num_nodes = instance->sink + 1;  // +1 porque ID parte en 0
    int *u = calloc(num_nodes, sizeof(int));
    int *visited = calloc(num_nodes, sizeof(int));

    if (!u || !visited) {
        fprintf(stderr, "Error de memoria en check_no_subtours.\n");
        free(u);
        free(visited);
        return 0;
    }

    int current = instance->source;
    u[current] = 0;
    visited[current] = 1;

    int order = 1;

    // Recorremos la solución paso a paso
    while (current != instance->sink) {
        int found = 0;
        for (int k = 0; k < instance->num_arcs; k++) {
            if (sol->arc_used[k] && instance->arcs[k].from.id == current) {
                int next = instance->arcs[k].to.id;

                if (visited[next]) {
                    DPRINT("Ciclo detectado: el nodo n%d ya fue visitado.\n", next + 1);
                    free(u);
                    free(visited);
                    return 0;
                }

                u[next] = order++;
                visited[next] = 1;
                current = next;
                found = 1;
                break;
            }
        }

        if (!found) {
            DPRINT("Ruta inconexa o terminación prematura en nodo n%d.\n", current + 1);
            free(u);
            free(visited);
            return 0;
        }
    }

    // Validar las restricciones MTZ reales
    for (int k = 0; k < instance->num_arcs; k++) {
        if (sol->arc_used[k]) {
            int i = instance->arcs[k].from.id;
            int j = instance->arcs[k].to.id;

            if (i != instance->source && j != instance->source && i != j) {
                int lhs = u[i] - u[j] + num_nodes;
                int rhs = num_nodes - 1;
                if (lhs > rhs) {
                    DPRINT("Violación MTZ: u[%d] - u[%d] + %d = %d > %d\n",
                           i + 1, j + 1, num_nodes, lhs, rhs);
                    free(u);
                    free(visited);
                    return 0;
                }
            }
        }
    }

    free(u);
    free(visited);
    return 1;
}

int check_resource_limit(const Instance *instance, const Solution *sol) {
    int total_resource = 0;

    for (int k = 0; k < instance->num_arcs; k++) {
        if (sol->arc_used[k]) {
            total_resource += instance->arcs[k].res;
        }
    }

    if (total_resource > instance->R_max) {
        DPRINT("Violación de límite de recurso: usado %d, máximo permitido %d\n",
               total_resource, instance->R_max);
        return 0;
    }

    return 1;
}

int check_no_forbidden_nodes(const Instance *instance, const Solution *sol) {
    for (int k = 0; k < instance->num_arcs; k++) {
        if (sol->arc_used[k]) {
            int from = instance->arcs[k].from.id;
            int to = instance->arcs[k].to.id;

            for (int f = 0; f < instance->num_forbidden; f++) {
                int forbidden_node = instance->forbidden[f];
                if (from == forbidden_node || to == forbidden_node) {
                    DPRINT("Arco usado (%d -> %d) pasa por nodo prohibido %d\n",
                           from + 1, to + 1, forbidden_node + 1);
                    return 0;
                }
            }
        }
    }
    return 1;
}

int check_required_nodes(const Instance *instance, const Solution *sol) {
    int num_required = instance->num_required;
    int *visited = calloc(instance->sink + 1, sizeof(int));

    if (!visited) {
        fprintf(stderr, "Error de memoria en check_required_nodes.\n");
        return 0;
    }

    // Marcar los nodos que reciben entrada
    for (int k = 0; k < instance->num_arcs; k++) {
        if (sol->arc_used[k]) {
            int to = instance->arcs[k].to.id;
            visited[to] = 1;
        }
    }

    for (int i = 0; i < num_required; i++) {
        int req = instance->required[i];
        if (!visited[req]) {
            DPRINT("Nodo requerido n%d no fue visitado.\n", req + 1);
            free(visited);
            return 0;
        }
    }

    free(visited);
    return 1;
}

int validate_time_windows(const Solution *sol, const Instance *inst, float *arrival_time) {
    int num_nodes = inst->sink + 1;
    int violations = 0;

    // Inicializar tiempos de llegada
    // printf("Inicializando tiempos de llegada para %d nodos.\n", num_nodes);
    // for (int i = 0; i < num_nodes; i++) {
    //     arrival_time[i] = 0.0f;
    // }

    for (int k = 0; k < sol->length; k++) {
        int arc_used = sol->arc_used_index[k];
        DPRINT("-------------------------\n");
        DPRINT("arco usado %d\n", arc_used);
        int from = inst->arcs[arc_used].from.id;
        int to = inst->arcs[arc_used].to.id;
        DPRINT("arco %d: %d -> %d, usado: %d\n", k, from + 1 , to + 1, arc_used);

        int to_start = inst->nodes[to].earliest;
        int to_end = inst->nodes[to].latest;
        DPRINT("ventana de tiempo del nodo %d: [%d, %d]\n", to + 1, to_start, to_end);

        float travel_time = inst->arcs[arc_used].time;
        arrival_time[to] = arrival_time[from] + travel_time;

        if (arrival_time[to] < to_start || arrival_time[to] > to_end) {
            DPRINT("❌ Violación en nodo %d: llegada %.2f fuera de [%d, %d]\n", to + 1, arrival_time[to], to_start, to_end);
            if (!already_recorded(to, sol->violated_nodes, violations)) {
                sol->violated_nodes[violations] = to;
                violations++;
            }
        } else {
            DPRINT("✅ Llegada válida a nodo %d: llegada %.2f dentro de [%d, %d]\n", to + 1, arrival_time[to], to_start, to_end);
        }
    }

    return violations;
}

bool already_recorded(int node, int *violated_nodes, int count) {
    for (int i = 0; i < count; i++) {
        if (violated_nodes[i] == node) {
            return true;
        }
    }
    return false;
}


