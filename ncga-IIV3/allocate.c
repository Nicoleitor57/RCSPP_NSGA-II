// /* Memory allocation and deallocation routines */

// # include <stdio.h>
// # include <stdlib.h>
// # include <math.h>

// # include "global.h"
// //# include "rand.h"
// # include "GraphReader.h"

// void solution_to_individual(Solution *sol, individual *ind, Instance *inst)
// {
//     evaluate_objective(inst, sol);
//     ind->sol = *sol;

//     int cont = 0;

//     //revisar restricciones sumar a contador por restriccion violada
//     if (check_flow_balance(inst, sol) == 0)
//     {   
//         printf("Restricción de flujo VIOLADA.\n");
//         cont++;
//     }
//     if (check_no_subtours(inst, sol) == 0)
//     {
//         printf("Restricción de subtours VIOLADA.\n");
//         cont++;
//     }
//     if (check_resource_limit(inst, sol) == 0)
//     {
//         printf("Restricción de recurso total VIOLADA.\n");
//         cont++;
//     }
//     if (check_no_forbidden_nodes(inst, sol) == 0)
//     {
//         printf("Restricción de nodos prohibidos VIOLADA.\n");
//         cont++;
//     }
//     if (check_required_nodes(inst, sol) == 0)
//     {
//         printf("Restricción de nodos requeridos VIOLADA.\n");
//         cont++;
//     }
//     int windows = validate_time_windows(sol, inst, sol->arrival_time);
//     if (windows != 0)
//     {
//         printf("Restricción de ventanas de tiempo VIOLADA.\n");
//         cont = cont + windows;
//     }

//     ind->constr_violation = cont;

// }

// void allocate_memory_pop (population *pop, int size)
// {
//     int i;
//     pop->ind = (individual *)malloc(size*sizeof(individual));
//     for (i=0; i<size; i++)
//     {
//         allocate_memory_ind (&(pop->ind[i]));
//     }
//     return;
// }

// void allocate_memory_ind (individual *ind)
// {
//     int j;
//     if (nreal != 0)
//     {
//         ind->xreal = (double *)malloc(nreal*sizeof(double));
//     }
//     if (nbin != 0)
//     {
//         ind->xbin = (double *)malloc(nbin*sizeof(double));
//         ind->gene = (int **)malloc(nbin*sizeof(int *));
//         for (j=0; j<nbin; j++)
//         {
//             ind->gene[j] = (int *)malloc(nbits[j]*sizeof(int));
//         }
//     }
//     ind->obj = (double *)malloc(nobj*sizeof(double));
//     if (ncon != 0)
//     {
//         ind->constr = (double *)malloc(ncon*sizeof(double));
//     }
//     return;
// }

/* Memory allocation and deallocation routines */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>

# include "global.h"
# include "rand.h"
#include <string.h>

void free_individual(individual *ind) {
    if (!ind) return;

    free(ind->gene);
    free(ind->obj);
    free(ind->constr);

    if (ind->sol) {
        free(ind->sol->arc_used);
        free(ind->sol->arc_used_index);
        free(ind->sol->violated_nodes);
        if (ind->sol->arrival_time) free(ind->sol->arrival_time);
        free(ind->sol);
    }
}



void solution_to_individual(Solution *sol, individual *ind, Instance *inst) {
    if (!sol || !ind || !inst) {
        fprintf(stderr, "Error: punteros nulos en solution_to_individual\n");
        exit(EXIT_FAILURE);
    }

    // Copiamos la ruta (genotipo) desde arc_used_index
    // ind->gene = calloc(sol->length, sizeof(int));
    // if (!ind->gene) {
    //     fprintf(stderr, "Error reservando memoria para gene\n");
    //     exit(EXIT_FAILURE);
    // }
    // for (int i = 0; i < sol->length; i++) {
    //     ind->gene[i] = sol->arc_used_index[i];
    // }
    memcpy(ind->gene[0], sol->arc_used_index, sol->length * sizeof(int));


    // Asignamos memoria para la copia profunda de sol
    ind->sol = malloc(sizeof(Solution));
    if (!ind->sol) {
        fprintf(stderr, "Error reservando memoria para ind->sol\n");
        exit(EXIT_FAILURE);
    }

    // Copia de atributos escalares
    ind->sol->length = sol->length;

    // Copia profunda de cada puntero (NO copiar punteros directamente)
    ind->sol->arc_used = calloc(inst->num_arcs, sizeof(int));
    ind->sol->arc_used_index = calloc(inst->num_arcs, sizeof(int));
    ind->sol->violated_nodes = calloc(inst->sink + 2, sizeof(int));
    ind->sol->arrival_time = NULL;  // ← Muy importante: se asignará más tarde en evaluate_solution

    if (!ind->sol->arc_used || !ind->sol->arc_used_index || !ind->sol->violated_nodes) {
        fprintf(stderr, "Error reservando memoria para campos de Solution\n");
        exit(EXIT_FAILURE);
    }

    // Copiar los valores
    memcpy(ind->sol->arc_used, sol->arc_used, inst->num_arcs * sizeof(int));
    memcpy(ind->sol->arc_used_index, sol->arc_used_index, inst->num_arcs * sizeof(int));
    memcpy(ind->sol->violated_nodes, sol->violated_nodes, (inst->sink + 2) * sizeof(int));

    // No copiar arrival_time aún — se recalcula en evaluate_solution
    ind->sol->arrival_time = NULL;

    // Opcional: inicializar objetivos y restricciones si no se hace antes
    //ind->obj = calloc(2, sizeof(double));
    //ind->constr = calloc(6, sizeof(double));
    //ind->constr_violation = 0.0;

    if (!ind->obj || !ind->constr) {
        fprintf(stderr, "Error reservando memoria para obj/constr\n");
        exit(EXIT_FAILURE);
    }
}



// void solution_to_individual(Solution *sol, individual *ind, Instance *inst){

//     if (sol->arc_used == NULL) {
//         fprintf(stderr, "ERROR: sol->arc_used es NULL\n");
//         exit(EXIT_FAILURE);
//     }
//     if (sol->length <= 0 || sol->length  > inst->num_arcs) {
//         fprintf(stderr, "ERROR: routhe_length=%d inválido\n", sol->length );
//         exit(EXIT_FAILURE);
//     }
//     int routhe_length = sol->length;
//     //ind->gene = malloc(sizeof(int *));
//     //ind->gene[0] = calloc(routhe_length, sizeof(int));
//     //ind->obj = calloc(2, sizeof(double));
//     //ind->constr = calloc(6, sizeof(double));
//     //ind->sol = malloc(sizeof(Solution));
//     memcpy(ind->gene[0], sol->arc_used, routhe_length*sizeof(int));
//     ind->sol = malloc(sizeof(Solution));
//     ind->sol->length = sol->length;
//     ind->sol->arc_used = calloc(inst->num_arcs, sizeof(int));
//     memcpy(ind->sol->arc_used, sol->arc_used, inst->num_arcs * sizeof(int));



    // ind->obj = calloc(2, sizeof(double));
    // ind->constr = calloc(6, sizeof(double));
    // int routhe_length = sol->length;
    // ind->gene = malloc(sizeof(int *));
    // ind->gene[0] = calloc(routhe_length, sizeof(int));

    // memcpy(ind->gene[0], sol->arc_used, routhe_length*sizeof(int));

    // int cont = 0;

    // //revisar restricciones sumar a contador por restriccion violada
    // if (check_flow_balance(inst, sol) == 0)
    // {   
    //     printf("Restricción de flujo VIOLADA.\n");
    //     cont++;
    //     ind->constr[0] = 1;
    // }else{
    //     ind->constr[0] = 0;
    // }

    // if (check_no_subtours(inst, sol) == 0)
    // {
    //     printf("Restricción de subtours VIOLADA.\n");
    //     cont++;
    //     ind->constr[1] = 1;
    // }else{
    //     ind->constr[1] = 0;
    // }

    // if (check_resource_limit(inst, sol) == 0)
    // {
    //     printf("Restricción de recurso total VIOLADA.\n");
    //     cont++;
    //     ind->constr[2] = 1;
    // }else{
    //     ind->constr[2] = 0;
    // }

    // if (check_no_forbidden_nodes(inst, sol) == 0)
    // {
    //     printf("Restricción de nodos prohibidos VIOLADA.\n");
    //     cont++;
    //     ind->constr[3] = 1;
    // }else{
    //     ind->constr[3] = 0;
    // }

    // if (check_required_nodes(inst, sol) == 0)
    // {
    //     printf("Restricción de nodos requeridos VIOLADA.\n");
    //     cont++;
    //     ind->constr[4] = 1;
    // }else{
    //     ind->constr[4] = 0;
    // }

    // //sol->arrival_time = malloc((sol->length)*sizeof(double));
    // if (sol->arrival_time) free(sol->arrival_time);
    // sol->arrival_time = calloc(sol->length, sizeof(double));


    // int windows = validate_time_windows(sol, inst, sol->arrival_time);
    // if (windows != 0)
    // {
    //     printf("Restricción de ventanas de tiempo VIOLADA.\n");
    //     cont = cont + windows;
    //     ind->constr[5] = windows;
    // }else{
    //     ind->constr[5] = 0;
    // }

    // ind->constr_violation = cont;

    // // printf("--------------------------------------\n"  );

    // // printf("Arcos usados en la solución:\n");
    // // for (int i = 0; i < sol->length; i++) {
    // //     printf(" %d", sol->arc_used_index[i]);
    // // }
    // // printf("\nLongitud: %d\n", sol->length);


    // double dist, time;
    // evaluate_objective(inst, sol, &dist, &time);

    // //printf("distancia: %f, tiempo: %f\n", dist, time);

    // ind->obj[0] = dist;
    // ind->obj[1] = time;
    // //printf("-------------------------\n"); 
    // //printf("distancia: %f, tiempo: %f\n", ind->obj[0], ind->obj[1]);
    // ind->sol = sol;
//}

/* Function to allocate memory to a population */
void allocate_memory_pop (population *pop, int size)
{
    int i;
    pop->ind = (individual *)malloc(size*sizeof(individual));
    for (i=0; i<size; i++)
    {
        allocate_memory_ind (&(pop->ind[i]));
    }
    return;
}

/* Function to allocate memory to an individual */
void allocate_memory_ind (individual *ind)
{
    int j;
    if (nreal != 0)
    {
        ind->xreal = (double *)malloc(nreal*sizeof(double));
    }
    if (nbin != 0)
    {
        ind->xbin = (double *)malloc(nbin*sizeof(double));
        ind->gene = (int **)malloc(nbin*sizeof(int *));
        for (j=0; j<nbin; j++)
        {
            //printf("nbits[%d] = %d\n", j, nbits[j]);
            //printf("nbin = %d\n", nbin);
            ind->gene[j] = (int *)malloc(nbits[j]*sizeof(int));
            //printf("se aloco bien");
        }
    }
    ind->obj = (double *)malloc(nobj*sizeof(double));
    if (ncon != 0)
    {
        ind->constr = (double *)malloc(ncon*sizeof(double));
    }
    return;
}

/* Function to deallocate memory to a population */
void deallocate_memory_pop (population *pop, int size)
{
    int i;
    for (i=0; i<size; i++)
    {
        deallocate_memory_ind (&(pop->ind[i]));
    }
    free (pop->ind);
    return;
}

/* Function to deallocate memory to an individual */
void deallocate_memory_ind (individual *ind)
{
    int j;
    if (nreal != 0)
    {
        free(ind->xreal);
    }
    if (nbin != 0)
    {
        for (j=0; j<nbin; j++)
        {
            free(ind->gene[j]);
        }
        free(ind->xbin);
        free(ind->gene);
    }
    free(ind->obj);
    if (ncon != 0)
    {
        free(ind->constr);
    }
    return;
}
