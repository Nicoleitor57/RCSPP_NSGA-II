/* Routine for evaluating population members  */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>

# include "global.h"
# include "rand.h"

/* Routine to evaluate objective function values and constraints for a population */
void evaluate_pop (population *pop, Instance *inst)
{
    int i;
    for (i=0; i<popsize; i++)
    {
        evaluate_ind (&(pop->ind[i]), inst);
        //printf("------------------------------------------------------\n");
        //printf("%d\n",i);
    }
    return;
}

/* Routine to evaluate objective function values and constraints for an individual */
/*void evaluate_ind (individual *ind)
{
    int j;
    test_problem (ind->xreal, ind->xbin, ind->gene, ind->obj, ind->constr);
    if (ncon==0)
    {
        ind->constr_violation = 0.0;
    }
    else
    {
        ind->constr_violation = 0.0;
        for (j=0; j<ncon; j++)
        {
            if (ind->constr[j]<0.0)
            {
                ind->constr_violation += ind->constr[j];
            }
        }
    }
    return;
}*/
void evaluate_solution(individual *ind, Instance *inst) {
    // ind->obj = malloc(2 * sizeof(double));
    // ind->constr = malloc(6 * sizeof(double));

    

    int violations = 0;
    Solution *sol = (ind->sol);  // Usamos la solución interna del individuo

    if (sol == NULL || sol->length <= 0 || sol->arc_used == NULL) {
        fprintf(stderr, "Evaluación ignorada: solución inválida (length=%d).\n", sol ? sol->length : -1);
        return;
    }

    // //revisar campos de sol
    // printf("Evaluando solución con longitud: %d\n", sol->length);
    // printf("Arcos usados: ");
    // for (int i = 0; i < sol->length; i++) {
    //     printf("%d ", sol->arc_used_index[i]);
    // }
    // printf("\n");
    // printf("Arcos usados (binario): ");
    // for (int i = 0; i < inst->num_arcs; i++) {
    //     printf("%d ", sol->arc_used[i]);
    // }
    // printf("\n");



    // Restricción 1: balance de flujo
    ind->constr[0] = (check_flow_balance(inst, sol) == 0) ? 1.0 : 0.0;
    violations += ind->constr[0];

    // Restricción 2: no subtours
    ind->constr[1] = (check_no_subtours(inst, sol) == 0) ? 1.0 : 0.0;
    violations += ind->constr[1];

    // Restricción 3: límite de recursos
    ind->constr[2] = (check_resource_limit(inst, sol) == 0) ? 1.0 : 0.0;
    violations += ind->constr[2];

    // Restricción 4: nodos prohibidos
    ind->constr[3] = (check_no_forbidden_nodes(inst, sol) == 0) ? 1.0 : 0.0;
    violations += ind->constr[3];

    // Restricción 5: nodos requeridos
    ind->constr[4] = (check_required_nodes(inst, sol) == 0) ? 1.0 : 0.0;
    violations += ind->constr[4];

    // Restricción 6: ventanas de tiempo
    if (sol->arrival_time) free(sol->arrival_time);
    //printf("sol->length: %d\n", sol->length);
    sol->arrival_time = calloc(inst->num_arcs, sizeof(double)); 

    int windows = validate_time_windows(sol, inst, sol->arrival_time);
    ind->constr[5] = (double) windows;
    violations += windows;

    ind->constr_violation = (double) violations;

    // Objetivos
    double dist, time;
    evaluate_objective(inst, sol, &dist, &time);
    ind->obj[0] = dist;
    ind->obj[1] = time;
    //printf("distancia: %f, tiempo: %f\n", ind->obj[0], ind->obj[1]);
}



/* Routine to evaluate objective function values and constraints for an individual */
void evaluate_ind (individual *ind, Instance *inst)
{
    /*Acá la evaluación completa. Deben setearse los valores de obj y constr_violation. */
    /*
    void test_problem (double *xreal, double *xbin, int **gene, double *obj, double *constr)
    {
    obj[0] = pow(xreal[0],2.0);
    obj[1] = pow((xreal[0]-2.0),2.0);
    return;
    }
    */

    //int j;
    /*test_problem (ind->xreal, ind->xbin, ind->gene, ind->obj, ind->constr);*/
    evaluate_solution(ind, inst);
    // printf("distancia: %f, tiempo: %f\n", ind->obj[0], ind->obj[1]);
    // printf("violaciones: %f\n", ind->constr_violation);
    // if (ncon==0)
    // {
    //     ind->constr_violation = 0.0;
    // }
    // else
    // {
    //     ind->constr_violation = 0.0;
    //     for (j=0; j<ncon; j++)
    //     {
    //         if (ind->constr[j]<0.0)
    //         {
    //             ind->constr_violation += ind->constr[j];
    //         }
    //     }
    // }
    // printf("violaciones 2: %f\n", ind->constr_violation);
    // return;
    
}
