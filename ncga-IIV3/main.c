#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "GraphReader.h"
#include "model.h"
#include "greedy.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <archivo_instancia>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int num_nodes = extract_num_nodes(filename);
    if (num_nodes < 0) {
        fprintf(stderr, "Error extrayendo número de nodos\n");
        return 1;
    }

    Instance instance = {0};
    convert_to_instance(filename, &instance);

    // Solution sol = {0};
    // sol.length = instance.num_arcs;
    // sol.arc_used = calloc(sol.length, sizeof(int));
    // sol.arc_used_index = calloc(sol.length, sizeof(int));

    // sol.arc_used[34] = 1; // Simulación de una solución
    // sol.arc_used[19] = 1; // Simulación de una solución
    // sol.arc_used[0] = 1; // Simulación de una solución

    // sol.arc_used_index[0] = 34;
    // sol.arc_used_index[1] = 19;
    // sol.arc_used_index[2] = 0;

    // sol.violated_nodes = calloc(sol.length, sizeof(int));

    // //contar arcos en sol.arc_used
    // int count = 0;
    // for (int i = 0; i < sol.length; i++) {
    //     if (sol.arc_used[i]) {
    //         count++;
    //     }
    // }
    // sol.length = count;

    // ObjectiveResult res = {0};

    // evaluate_objective(&instance, &sol, &res);

    // printf("Distancia total: %.2f\n", res.objectives[0]);
    // printf("Tiempo total: %.2f\n", res.objectives[1]);

    //   // Verificar la restricción de flujo
    // if (check_flow_balance(&instance, &sol)) {
    //     printf("Restricción de flujo satisfecha.\n");
    // } else {
    //     printf("Restricción de flujo VIOLADA.\n");
    // }

    // if (check_no_subtours(&instance, &sol)) {
    //     printf("Restricción de subtours satisfecha.\n");
    // } else {
    //     printf("Restricción de subtours VIOLADA.\n");
    // }

    // if (check_resource_limit(&instance, &sol)) {
    //     printf("Restricción de recurso total satisfecha.\n");
    // } else {
    //     printf("Restricción de recurso total VIOLADA.\n");
    // }

    // if (check_no_forbidden_nodes(&instance, &sol)) {
    //     printf("Restricción de nodos prohibidos satisfecha.\n");
    // } else {
    //     printf("Restricción de nodos prohibidos VIOLADA.\n");
    // }

    // if (check_required_nodes(&instance, &sol)) {
    //     printf("Restricción de nodos requeridos satisfecha.\n");
    // } else {
    //     printf("Restricción de nodos requeridos VIOLADA.\n");
    // }


    // float *arrival_times = malloc((sol.length) * sizeof(float));
    // int vialoation = validate_time_windows(&sol, &instance, arrival_times);

    // printf("\nTotal de violaciones: %d\n", vialoation);
    // for (int i = 0; i < vialoation; i++) {
    //     int node = sol.violated_nodes[i];
    //     printf("Nodo con violación: %d | Tiempo de llegada: %.2f | Ventana esperada: [%d, %d]\n",
    //         node + 1,                              // +1 para mostrarlo con índice humano
    //         arrival_times[node],                   // Tiempo real de llegada
    //         instance.nodes[node].earliest,             // Ventana de tiempo esperada
    //         instance.nodes[node].latest);
    // }

    
    // free(arrival_times);

    // free(sol.arc_used);
    // free(instance.nodes);
    // free(instance.arcs);
    // free(instance.required);
    // free(instance.forbidden);
    // free(sol.arrival_time);
    
    // return 0;

    int num_solutions = 5;
    Solution *solutions = calloc(num_solutions, sizeof(Solution));

    for (int i = 0; i < num_solutions; i++) {
        solutions[i].arc_used = calloc(instance.num_arcs, sizeof(int));
        solutions[i].arc_used_index = calloc(instance.num_arcs, sizeof(int));
        solutions[i].violated_nodes = calloc(instance.sink + 2, sizeof(int));
    }

    generate_initial_solutions(&instance, solutions, num_solutions);

    //mostrar soluciones
    for (int i = 0; i < num_solutions; i++) {
        printf("Solución %d:\n", i + 1);
        for (int j = 0; j < instance.num_arcs; j++) {
            if (solutions[i].arc_used[j]) {
                //printf("Arco usado: %d -> %d\n", instance.arcs[j].from.id + 1, instance.arcs[j].to.id + 1);
                printf("De nodo %d a nodo %d\n", instance.arcs[j].from.id + 1, instance.arcs[j].to.id + 1);
            }
        }
        printf("\n");
    }

    // Liberar memoria
    for (int i = 0; i < num_solutions; i++) {
        free(solutions[i].arc_used);
        free(solutions[i].arc_used_index);
        free(solutions[i].violated_nodes);
    }
    free(solutions);

    return 0;
}
