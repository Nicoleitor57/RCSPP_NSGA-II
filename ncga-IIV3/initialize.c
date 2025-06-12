/* Data initializtion routines */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>

# include "global.h"
# include "rand.h"

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
    ind->gene = malloc(nbin * sizeof(int *));
    for (int j = 0; j < nbin; j++) {
        ind->gene[j] = calloc(nbits[j], sizeof(int)); // usa calloc para inicializar en 0
    }

    // Ahora puedes usar ind->gene[j][k]
    for (int j = 0; j < nbin; j++) {
        for (int k = 0; k < nbits[j]; k++) {
            ind->gene[j][k] = (randomperc() < 0.5) ? 0 : 1;
        }
    }
    
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
                    ind->gene[j][k] = 0;
                }
                else
                {
                    ind->gene[j][k] = 1;
                }
            }
        }
    }
    return;
}
