/* Routines for storing population data into files */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>

# include "global.h"
# include "rand.h"

/* Function to print the information of a population in a file */
// --- VERSIÓN CORREGIDA DE LA FUNCIÓN DE REPORTE ---
// Genera reportes de población con un formato legible y unificado.
void report_pop(population *pop, FILE *fpt) {
    int i, j;

    for (i = 0; i < popsize; i++) {
        // --- Encabezado para cada individuo ---
        fprintf(fpt, "# Individual %d\n", i + 1);

        // --- Objetivos ---
        fprintf(fpt, "Objectives: ");
        for (j = 0; j < nobj; j++) {
            // Usamos %g para un formato más compacto y legible de los doubles
            fprintf(fpt, "%g\t", pop->ind[i].obj[j]);
        }
        fprintf(fpt, "\n");

        // --- Violación de Restricciones ---
        fprintf(fpt, "Constraint Violation: %g\n", pop->ind[i].constr_violation);

        // --- Rank y Crowding Distance ---
        fprintf(fpt, "Rank: %d\t Crowding Distance: %g\n", pop->ind[i].rank, pop->ind[i].crowd_dist);

        // --- Genes (el camino de arcos) ---
        fprintf(fpt, "Path (arc indices): ");
        j = 0;
        while (j < nbits[0] && pop->ind[i].gene[0][j] != -1) {
            fprintf(fpt, "%d ", pop->ind[i].gene[0][j]);
            j++;
        }
        fprintf(fpt, "\n");

        // --- Separador para mayor claridad ---
        fprintf(fpt, "--------------------------------------------------\n");
    }
}

/* Function to print the information of feasible and non-dominated population in a file */
void report_feasible (population *pop, FILE *fpt)
{
    int i, j, k;
    for (i=0; i<popsize; i++)
    {
        if (pop->ind[i].constr_violation == 0.0 && pop->ind[i].rank==1)
        {
            for (j=0; j<nobj; j++)
            {
                fprintf(fpt,"%e\t",pop->ind[i].obj[j]);
            }
            if (ncon!=0)
            {
                for (j=0; j<ncon; j++)
                {
                    fprintf(fpt,"%e\t",pop->ind[i].constr[j]);
                }
            }
            if (nreal!=0)
            {
                for (j=0; j<nreal; j++)
                {
                    fprintf(fpt,"%e\t",pop->ind[i].xreal[j]);
                }
            }
            if (nbin!=0)
            {
                for (j=0; j<nbin; j++)
                {
                    for (k=0; k<nbits[j]; k++)
                    {
                        fprintf(fpt,"%d\t",pop->ind[i].gene[j][k]);
                    }
                }
            }
            fprintf(fpt,"%e\t",pop->ind[i].constr_violation);
            fprintf(fpt,"%d\t",pop->ind[i].rank);
            fprintf(fpt,"%e\n",pop->ind[i].crowd_dist);
        }
    }
    return;
}
