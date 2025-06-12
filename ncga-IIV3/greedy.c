#define DEBUG_PRINTS 0
#if DEBUG_PRINTS
#define DPRINT(...) printf(__VA_ARGS__)
#else
#define DPRINT(...)
#endif
#include "greedy.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Mezcla aleatoria de un arreglo
void shuffle(int *array, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

// Verifica si un nodo ya está en la solución (para evitar ciclos)
int is_in_path(int node, const Solution *sol, const Instance *inst) {
    for (int i = 0; i < sol->length; i++) {
        int from = inst->arcs[sol->arc_used_index[i]].from.id;
        int to = inst->arcs[sol->arc_used_index[i]].to.id;
        if (node == from || node == to) return 1;
    }
    return 0;
}

// Genera múltiples soluciones aleatorias válidas
void generate_initial_solutions(const Instance *inst, Solution *solutions, int num_solutions) {
    srand(time(NULL));  // Semilla para aleatoriedad

    int progress_step = num_solutions / 10;
    if (progress_step == 0) progress_step = 1;

    for (int i = 0; i < num_solutions; i++) {
        Solution *sol = &solutions[i];
        sol->length = 0;
        for (int j = 0; j < inst->num_arcs; j++) {
            sol->arc_used[j] = 0;
        }

        int current = inst->source;
        int steps = 0;

        while (current != inst->sink && steps < inst->num_arcs) {
            int *candidate_indices = (int *)malloc(inst->num_arcs * sizeof(int));
            if (!candidate_indices) {
                fprintf(stderr, "Error: No se pudo asignar memoria para candidate_indices\n");
                exit(EXIT_FAILURE);
            }
            int count = 0;

            // Recolectar arcos válidos desde el nodo actual
            for (int k = 0; k < inst->num_arcs; k++) {
                if (inst->arcs[k].from.id != current) continue;

                int to = inst->arcs[k].to.id;

                // Verificar nodo prohibido
                int forbidden = 0;
                for (int f = 0; f < inst->num_forbidden; f++) {
                    if (to == inst->forbidden[f]) {
                        forbidden = 1;
                        break;
                    }
                }
                if (forbidden) continue;

                // Evitar ciclos
                if (is_in_path(to, sol, inst)) continue;

                candidate_indices[count++] = k;
            }

            if (count == 0) {
                DPRINT("No hay candidatos válidos desde el nodo %d. Abortando ruta.\n", current + 1);
                free(candidate_indices);
                break;
            }

            shuffle(candidate_indices, count);
            int chosen_arc = candidate_indices[0];

            sol->arc_used[chosen_arc] = 1;
            sol->arc_used_index[sol->length++] = chosen_arc;
            
            current = inst->arcs[chosen_arc].to.id;
            steps++;

            free(candidate_indices);
        }

        // Validación de solución fuera del while
        //float *arrival_times = (float *)malloc((inst->sink + 2) * sizeof(float));
        // if (!arrival_times) {
        //     fprintf(stderr, "Error: No se pudo asignar memoria para arrival_times\n");
        //     exit(EXIT_FAILURE);
        // }

        if (!check_flow_balance(inst, sol) ||
            !check_no_subtours(inst, sol) ||
            !check_resource_limit(inst, sol) ||
            !check_no_forbidden_nodes(inst, sol) ||
            !check_required_nodes(inst, sol) ){
            //validate_time_windows(sol, inst, arrival_times) > 0) {
            DPRINT("⚠️ Solución %d no válida, se descartará\n", i + 1);
            //sol->length = 0;
        } else {
            DPRINT("✅ Solución %d generada con éxito\n", i + 1);
        }

        //free(arrival_times);

        // Barra de progreso
        if ((i + 1) % progress_step == 0 || i == num_solutions - 1) {
            int progress = (100 * (i + 1)) / num_solutions;
            DPRINT("\rProgreso: [%-50.*s] %3d%%", progress / 2, "##################################################\n", progress);
            fflush(stdout);
        }
        
    }
    DPRINT("\n"); // Salto de línea al final de la barra de progreso
}
