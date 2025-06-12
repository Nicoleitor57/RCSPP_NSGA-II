
#define DEBUG_PRINTS 0
#if DEBUG_PRINTS
#define DPRINT(...) printf(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <time.h>


# include "global.h"
# include "rand.h"



// --- VERSIÓN CORREGIDA DE LA FUNCIÓN DE EVALUACIÓN ---
void evaluate_objective_g(const Instance *instance, individual *ind) {
    float total_dist = 0.0f;
    float total_time = 0.0f;
    // Puedes añadir más variables para otros objetivos si es necesario (ej. total_risk)

    // 1. Encontrar la longitud del camino en el gen
    int solution_length = 0;
    while (solution_length < nbits[0] && ind->gene[0][solution_length] != -1) {
        solution_length++;
    }

    // Si el camino no tiene arcos, es inválido.
    if (solution_length == 0) {
        ind->obj[0] = 1e9; // Asignar un valor de penalización muy alto
        ind->obj[1] = 1e9;
        return;
    }

    // 2. Recorrer el camino REAL y sumar los objetivos
    for (int i = 0; i < solution_length; i++) {
        // --- LA CORRECCIÓN CLAVE ESTÁ AQUÍ ---
        // Obtenemos el índice del arco desde el gen del individuo
        int arc_idx = ind->gene[0][i];

        // Usamos 'arc_idx' para acceder a la información del arco correcto
        total_dist += (float)instance->arcs[arc_idx].dist;
        total_time += (float)instance->arcs[arc_idx].time;
        
        // NOTA: La lógica de 'risk_multiplier' y 'epsilon' era confusa
        // y probablemente incorrecta. Por ahora, se elimina para tener un
        // cálculo base que sea 100% correcto. Podemos añadir un cálculo
        // de riesgo adecuado después si es necesario.
    }

    // 3. Asignar los valores calculados a los objetivos del individuo
    ind->obj[0] = total_dist;
    ind->obj[1] = total_time;

    // NOTA: Aquí también deberías calcular la violación de restricciones (como 'total_res')
    // y asignarla a 'ind->constr_violation' para que NSGA-II funcione correctamente.
}

int check_flow_balance_g(const Instance *instance, const individual *ind) {
    int num_nodes = instance->sink + 1;  // IDs van de 0 a sink
    // Array para llevar cuenta del flujo en cada nodo
    int *flow = calloc(num_nodes, sizeof(int));
    if (!flow) {
        fprintf(stderr, "Error de memoria en check_flow_balance.\n");
        return 0;
    }

    //assert(instance->num_arcs > 0);
    //assert(sol->arc_used != NULL);

    // Recorremos los arcos y verificamos los usados en la solución
    for (int k = 0; k < instance->num_arcs; k++) {
        // printf("Verificando arco %d: %d -> %d, usado: %d\n",
        //        k, instance->arcs[k].from.id, instance->arcs[k].to.id, sol->arc_used[k]);
        if (ind->gene[1][k]) {  // Usamos gene[1] como el vector de arcos usados
            int from_id = instance->arcs[k].from.id;
            int to_id = instance->arcs[k].to.id;

            //assert(from_id >= 0 && from_id < num_nodes);
            //assert(to_id >= 0 && to_id < num_nodes);

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

int check_no_subtours_g(const Instance *instance, const individual *ind) {
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
            if (ind->gene[1][k] && instance->arcs[k].from.id == current) {
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
        if (ind->gene[1][k]) {  // Verificar si el arco está usado
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

int check_resource_limit_g(const Instance *instance, const individual *ind) {
    int total_resource = 0;

    for (int k = 0; k < instance->num_arcs; k++) {
        if (ind->gene[1][k]) {  // Usamos gene[1] como el vector de arcos usados
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

int check_no_forbidden_nodes_g(const Instance *instance, const individual *ind) {
    for (int k = 0; k < instance->num_arcs; k++) {
        if (ind->gene[1][k]) {  // Usamos gene[1] como el vector de arcos usados
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

int check_required_nodes_g(const Instance *instance, const individual *ind) {
    int num_required = instance->num_required;
    if (num_required == 0) return 1; // Si no hay nodos requeridos, la restricción se cumple

    int num_nodes = instance->sink + 1;
    int *visited_nodes = calloc(num_nodes, sizeof(int));
    if (!visited_nodes) {
        fprintf(stderr, "Error de memoria en check_required_nodes.\n");
        return 0;
    }

    // Marcar todos los nodos que están en el camino
    int path_len = 0;
    while(path_len < nbits[0] && ind->gene[0][path_len] != -1) {
        int arc_idx = ind->gene[0][path_len];
        // MARCAR TANTO EL INICIO COMO EL FIN DEL ARCO
        visited_nodes[instance->arcs[arc_idx].from.id] = 1;
        visited_nodes[instance->arcs[arc_idx].to.id] = 1;
        path_len++;
    }

    // Ahora, verificar si todos los requeridos fueron visitados
    for (int i = 0; i < num_required; i++) {
        int req_node = instance->required[i];
        if (req_node < num_nodes && !visited_nodes[req_node]) {
            DPRINT("Nodo requerido n%d no fue visitado.\n", req_node);
            free(visited_nodes);
            return 0; // Falla la validación
        }
    }

    free(visited_nodes);
    return 1; // Éxito
}

// Retorna la suma total de los retrasos (violación de la ventana de tiempo "latest")
int validate_time_windows_g(const individual *ind, const Instance *inst) {
    double total_lateness = 0.0;
    double current_time = 0.0; // El tiempo de llegada al nodo fuente es 0

    // Usamos un arreglo temporal para no modificar el gen del individuo durante la evaluación
    double arrival_times[inst->sink + 1];
    arrival_times[inst->source] = 0.0;

    int path_len = 0;
    while (path_len < nbits[0] && ind->gene[0][path_len] != -1) {
        int arc_idx = ind->gene[0][path_len];
        int from_node = inst->arcs[arc_idx].from.id;
        int to_node = inst->arcs[arc_idx].to.id;

        // Hora de salida del nodo 'from'
        // Es la hora de llegada o la apertura de la ventana, lo que sea más tarde
        double departure_time_from = fmax(arrival_times[from_node], (double)inst->nodes[from_node].earliest);
        
        // Hora de llegada al nodo 'to'
        double arrival_time_to = departure_time_from + (double)inst->arcs[arc_idx].time;
        arrival_times[to_node] = arrival_time_to;

        // Comprobar si la llegada a 'to' viola el límite 'latest'
        if (arrival_time_to > (double)inst->nodes[to_node].latest) {
            total_lateness += arrival_time_to - (double)inst->nodes[to_node].latest;
        }
        path_len++;
    }
    return total_lateness;
}

bool already_recorded_g(int node, individual *ind, int count) {
    for (int i = 0; i < count; i++) {
        if (ind->gene[3][i] == node) {
            return true;
        }
    }
    return false;
}


