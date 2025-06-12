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
    // Restricción 1: balance de flujo
    ind->constr[0] = (check_flow_balance_g(inst, ind) == 0) ? 1.0 : 0.0;
    violations += ind->constr[0];

    // Restricción 2: no subtours
    ind->constr[1] = (check_no_subtours_g(inst, ind) == 0) ? 1.0 : 0.0;
    violations += ind->constr[1];

    // Restricción 3: límite de recursos
    ind->constr[2] = (check_resource_limit_g(inst, ind) == 0) ? 1.0 : 0.0;
    violations += ind->constr[2];

    // Restricción 4: nodos prohibidos
    ind->constr[3] = (check_no_forbidden_nodes_g(inst, ind) == 0) ? 1.0 : 0.0;
    violations += ind->constr[3];

    // Restricción 5: nodos requeridos
    //ind->constr[4] = (check_required_nodes_g(inst, ind) == 0) ? 1.0 : 0.0;
    //violations += ind->constr[4];

    // Restricción 6: ventanas de tiempo

    int windows = validate_time_windows_g(ind, inst);
    ind->constr[5] = (double) windows;
    violations += windows;

    ind->constr_violation = (double) violations;

    // Objetivos
    
    evaluate_objective_g(inst, ind);
    //printf("distancia: %f, tiempo: %f\n", ind->obj[0], ind->obj[1]);
}



/* Routine to evaluate objective function values and constraints for an individual */
// =============================================================================
// FUNCIÓN DE EVALUACIÓN CON PENALIZACIONES PONDERADAS
// =============================================================================

// --- Pesos de Penalización (fáciles de ajustar aquí) ---
#define PENALTY_BROKEN_PATH 300.0      // Falla catastrófica: camino roto o con ciclos.
#define PENALTY_FORBIDDEN_NODE 50.0  // Penalización fija por usar un nodo prohibido.
#define PENALTY_REQUIRED_NODE 50.0   // Penalización fija por omitir un nodo requerido.
#define PENALTY_RESOURCE_BASE 10.0   // Penalización base por exceder los recursos.
// NOTA: Para Time Windows, la penalización será la magnitud del retraso.

void evaluate_ind(individual *ind, Instance *inst) {
    
    // --- PASO 1: VALIDACIÓN DE FACTIBILIDAD ESTRUCTURAL (FALLO CATASTRÓFICO) ---
    // Si la solución no es un camino válido de source a sink, es inútil.
    if (!check_flow_balance_g(inst, ind) || !check_no_subtours_g(inst, ind)) {
        ind->obj[0] = PENALTY_BROKEN_PATH;
        ind->obj[1] = PENALTY_BROKEN_PATH;
        ind->constr_violation = PENALTY_BROKEN_PATH;
        return; // Salida inmediata. No hay nada más que evaluar.
    }

    // --- PASO 2: LA SOLUCIÓN ES UN CAMINO VÁLIDO. CALCULAR OBJETIVOS. ---
    // Si llegamos aquí, sabemos que la solución es un camino completo de source a sink.
    float total_dist = 0.0f;
    float total_time = 0.0f;
    int total_res = 0;

    int solution_length = 0;
    while (solution_length < nbits[0] && ind->gene[0][solution_length] != -1) {
        int arc_idx = ind->gene[0][solution_length];
        total_dist += (float)inst->arcs[arc_idx].dist;
        total_time += (float)inst->arcs[arc_idx].time;
        total_res  += inst->arcs[arc_idx].res;
        solution_length++;
    }

    // Asignar los valores de objetivo base.
    ind->obj[0] =+ total_dist;
    ind->obj[1] =+ total_time;
    
    // --- PASO 3: ACUMULAR VIOLACIONES DE RESTRICCIONES "SUAVES" CON PESOS ---
    double total_violation = 0.0;
    
    // 3.1: Violación de límite de recursos
    if (total_res > inst->R_max) {
        // La penalización es la base + la cantidad por la que se excedió.
        total_violation += PENALTY_RESOURCE_BASE + (double)(total_res - inst->R_max);
    }
    
    // 3.2: Violación de nodos prohibidos
    if (!check_no_forbidden_nodes_g(inst, ind)) {
        total_violation += PENALTY_FORBIDDEN_NODE;
    }
    
    // 3.3: Violación de nodos requeridos
    if (!check_required_nodes_g(inst, ind)) { // Usando la versión corregida de esta función
        total_violation += PENALTY_REQUIRED_NODE;
    }
    
    // 3.4: Violación de ventanas de tiempo
    // Usamos la magnitud del retraso como penalización, lo cual es muy informativo.
    // Si quieres usar un peso fijo como 25, puedes cambiar la línea a:
    // if (validate_time_windows_g(ind, inst) > 0) { total_violation += 25.0; }
    total_violation += validate_time_windows_g(ind, inst); // Usando la versión robusta que devuelve el retraso total

    // Asignar la suma ponderada de todas las violaciones.
    ind->constr_violation =+ total_violation;
}


