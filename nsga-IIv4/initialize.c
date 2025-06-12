/* Data initializtion routines */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <string.h>

# include "global.h"
# include "rand.h"
# include "model.h"


/* Function to initialize a population randomly */
void initialize_pop (population *pop)
{
    int i;
    for (i=0; i<popsize; i++)
    {
        initialize_ind (&(pop->ind[i]));
    }
    return;
}

/* Function to initialize an individual randomly */
void initialize_ind (individual *ind)
{
    int j, k;
    if (nreal!=0)
    {
        for (j=0; j<nreal; j++)
        {
            ind->xreal[j] = rndreal (min_realvar[j], max_realvar[j]);
        }
    }
    if (nbin!=0)
    {
        for (j=0; j<nbin; j++)
        {
            for (k=0; k<nbits[j]; k++)
            {
                if (randomperc() <= 0.5)
                {
                    ind->gene[j][k] = -1;
                }
                else
                {
                    ind->gene[j][k] = -1;
                }
            }
        }
    }
    return;
}

void solution_to_individual(Solution *sol, individual *ind, Instance *inst) {
    if (sol == NULL || ind == NULL || inst == NULL) {
        fprintf(stderr, "Error: solución o individuo nulo.\n");
        return;
    }

    ind->sol = sol; // Asignar la solución al individuo
    ind->rank = 0; // Inicializar el rango
    ind->constr_violation = 0.0; // Inicializar violación de restricciones

    // Asignar los genes del individuo basados en la solución
    // for (int i = 0; i < nbin; i++) {
    //     for (int j = 0; j < nbits[i]; j++) {
    //         ind->gene[i][j] = sol->arc_used[j]; // Asignar arcos usados
    //     }
    // }
    memcpy(ind->gene[0], sol->arc_used_index, sizeof(int) * sol->length);
    //printf("--------------------------------------------\n");

    // //impprimir sol->arc_used
    // for (int i = 0; i < inst->num_arcs; i++) {
    //     if (sol->arc_used[i]) {
    //         printf("arco %d: %d -> %d, usado: %d\n", i, inst->arcs[i].from.id, inst->arcs[i].to.id, sol->arc_used[i]);
    //     }
    // }

}