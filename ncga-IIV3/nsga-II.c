#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "GraphReader.h"
#include "model.h"
#include "greedy.h"
#include "global.h"
#include "rand.h"

int nreal = 0;    // No usas variables reales para RCSPP
int nbin = 1;     // Solo un bloque de genes (para la ruta)
int *nbits = NULL; // Se inicializa más adelante
int nobj = 2;     // Dos objetivos: distancia y tiempo
int ncon = 6;     // Seis restricciones: flujo, subtours, recursos, nodos prohibidos, nodos requeridos, ventanas de tiempo


double pmut_real = 0.1;
int nbincross = 0;
double pcross_bin = 0.9;
int nrealcross = 0;
double pcross_real = 0.9;
double eta_c = 20.0;
int popsize; // Tamaño de la población
double seed; // Semilla para la aleatoriedad
double *min_realvar;
double *max_realvar;
double *min_binvar;
double *max_binvar;
int ngen;

FILE *fpt1;
FILE *fpt2;
FILE *fpt3;
FILE *fpt4;
FILE *fpt5;

int main(int argc, char *argv[]) {

    fpt1 = fopen("initial_pop.out","w");
    fpt2 = fopen("final_pop.out","w");
    fpt3 = fopen("best_pop.out","w");
    fpt4 = fopen("all_pop.out","w");
    fpt5 = fopen("params.out","w");
    fprintf(fpt1,"# This file contains the data of initial population\n");
    fprintf(fpt2,"# This file contains the data of final population\n");
    fprintf(fpt3,"# This file contains the data of final feasible population (if found)\n");
    fprintf(fpt4,"# This file contains the data of all generations\n");
    fprintf(fpt5,"# This file contains information about inputs as read by the program\n");
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <archivo_instancia> <tamano_poblacion>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    popsize = atoi(argv[2]);
    seed = atoi(argv[3]);
    ngen = atoi(argv[4]);

    int num_nodes = extract_num_nodes(filename);
    if (num_nodes < 0) {
        fprintf(stderr, "Error extrayendo número de nodos\n");
        return 1;
    }

    Instance instance = {0};
    convert_to_instance(filename, &instance);

    int num_solutions = popsize;
    Solution *solutions = calloc(num_solutions, sizeof(Solution));
    if (!solutions) {
        fprintf(stderr, "Error reservando memoria para soluciones\n");
        return 1;
    }

    for (int i = 0; i < num_solutions; i++) {
        solutions[i].arc_used = calloc(instance.num_arcs, sizeof(int));
        solutions[i].arc_used_index = calloc(instance.num_arcs, sizeof(int));
        solutions[i].violated_nodes = calloc(instance.sink + 2, sizeof(int));
        if (!solutions[i].arc_used || !solutions[i].arc_used_index || !solutions[i].violated_nodes) {
            fprintf(stderr, "Error reservando memoria para solución %d\n", i);
            return 1;
        }
    }

    generate_initial_solutions(&instance, solutions, num_solutions);

    // //printf solutiions length
    // for (int i = 0; i < num_solutions; i++) {
    //     printf("Solución %d - Longitud: %d\n", i, solutions[i].length);
        
    // }

    printf("Soluciones generadas:\n");

    // Tamaño máximo del gene: camino máximo esperado
    nbits = malloc(sizeof(int) * nbin);
    nbits[0] = num_nodes * 7;

    population *parent_pop = (population *)malloc(sizeof(population));
    population *child_pop = (population *)malloc(sizeof(population));
    population *mixed_pop = (population *)malloc(sizeof(population));

    if (!parent_pop || !child_pop || !mixed_pop) {
        fprintf(stderr, "Error reservando memoria para poblaciones\n");
        return 1;
    }

    if (nbits == NULL) {
        printf("ERROR: nbits es NULL antes de allocate_memory_pop\n");
        exit(1);
    }
    if (nbin <= 0 || nbits[0] <= 0) {
        printf("ERROR: nbin=%d o nbits[0]=%d inválidos\n", nbin, nbits[0]);
        exit(1);
    }


    allocate_memory_pop(parent_pop, popsize);
    allocate_memory_pop(child_pop, popsize);
    allocate_memory_pop(mixed_pop, 2 * popsize);
    

    if (!parent_pop->ind || !child_pop->ind || !mixed_pop->ind) {
        fprintf(stderr, "Error: memoria no asignada correctamente a las poblaciones\n");
        return 1;
    }

    for (int i = 0; i < num_solutions && i < popsize; i++) {
        solution_to_individual(&solutions[i], &parent_pop->ind[i], &instance);
    }

    // for (int i = num_solutions; i < popsize; i++) {
    //     printf("Individuo %d aún no ha sido inicializado.\n", i);
    // }

    randomize();
    initialize_pop (parent_pop);

    printf("\n Initialization done, now performing first generation\n");

    // for (int i = 0; i < popsize; i++) {
    //     printf("Individuo %d - Primer gen: %d\n", i, parent_pop->ind[i].gene[0][0]);
    // }

    //decode_pop(parent_pop);
    evaluate_pop (parent_pop, &instance);

    assign_rank_and_crowding_distance (parent_pop);
    // printf("Población inicial:\n");
    // for (int i = 0; i < popsize; i++) {
    //     printf("Individuo %d - Objetivos: [%.2f, %.2f], Rank: %d, Crowding Distance: %.2f\n",
    //            i,
    //            parent_pop->ind[i].obj[0],
    //            parent_pop->ind[i].obj[1],
    //            parent_pop->ind[i].rank,
    //            parent_pop->ind[i].crowd_dist);
    // }

    // printf("Individuo %d - Rank: %d, Crowding Distance: %.2f\n",
    //         1,
    //         parent_pop->ind[1].rank,
    //         parent_pop->ind[1].crowd_dist);
    // //rank and crowding distance
    // for (int i = 0; i < popsize; i++) {
    //     printf("Individuo %d - Rank: %d, Crowding Distance: %.2f\n",
    //         i,
    //         parent_pop->ind[i].rank,
    //         parent_pop->ind[i].crowd_dist);
    // }
    fflush(stdout);

    report_pop (parent_pop, fpt1);
    fprintf(fpt4,"# gen = 1\n");
    report_pop(parent_pop,fpt4);
    printf("\n gen = 1");
    fflush(stdout);

    fflush(fpt1);
    fflush(fpt2);
    fflush(fpt3);
    fflush(fpt4);
    fflush(fpt5);
    
    int i;
    for (i=2; i<=ngen; i++)
    {
        selection (parent_pop, child_pop);
        mutation_pop (child_pop, &instance);
    }



    for (int i = 0; i < num_solutions; i++) {
        free(solutions[i].arc_used);
        free(solutions[i].arc_used_index);
        free(solutions[i].violated_nodes);
    }
    free(solutions);

    for (int i = 0; i < popsize; i++) {
        for (int j = 0; j < nbin; j++) {
            free(parent_pop->ind[i].gene[j]);
        }
        free(parent_pop->ind[i].gene);
    }

    for (int i = 0; i < popsize; i++) {
        for (int j = 0; j < nbin; j++) {
            free(child_pop->ind[i].gene[j]);
        }
        free(child_pop->ind[i].gene);
    }

    for (int i = 0; i < 2 * popsize; i++) {
        for (int j = 0; j < nbin; j++) {
            free(mixed_pop->ind[i].gene[j]);
        }
        free(mixed_pop->ind[i].gene);
    }

    free(parent_pop->ind);
    free(child_pop->ind);
    free(mixed_pop->ind);
    free(parent_pop);
    free(child_pop);
    free(mixed_pop);
    free(nbits);

    return 0;
}
