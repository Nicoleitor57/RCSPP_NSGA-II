/* Crossover routines */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>

# include "global.h"
# include "rand.h"

/* Function to cross two individuals */
void crossover (individual *parent1, individual *parent2, individual *child1, individual *child2, Instance *instance)
{
    if (nreal!=0)
    {
        realcross (parent1, parent2, child1, child2);
    }
    if (nbin!=0)
    {
        bincross (parent1, parent2, child1, child2, instance);
    }
    return;
}

/* Routine for real variable SBX crossover */
void realcross (individual *parent1, individual *parent2, individual *child1, individual *child2)
{
    int i;
    double rand;
    double y1, y2, yl, yu;
    double c1, c2;
    double alpha, beta, betaq;
    if (randomperc() <= pcross_real)
    {
        nrealcross++;
        for (i=0; i<nreal; i++)
        {
            if (randomperc()<=0.5 )
            {
                if (fabs(parent1->xreal[i]-parent2->xreal[i]) > EPS)
                {
                    if (parent1->xreal[i] < parent2->xreal[i])
                    {
                        y1 = parent1->xreal[i];
                        y2 = parent2->xreal[i];
                    }
                    else
                    {
                        y1 = parent2->xreal[i];
                        y2 = parent1->xreal[i];
                    }
                    yl = min_realvar[i];
                    yu = max_realvar[i];
                    rand = randomperc();
                    beta = 1.0 + (2.0*(y1-yl)/(y2-y1));
                    alpha = 2.0 - pow(beta,-(eta_c+1.0));
                    if (rand <= (1.0/alpha))
                    {
                        betaq = pow ((rand*alpha),(1.0/(eta_c+1.0)));
                    }
                    else
                    {
                        betaq = pow ((1.0/(2.0 - rand*alpha)),(1.0/(eta_c+1.0)));
                    }
                    c1 = 0.5*((y1+y2)-betaq*(y2-y1));
                    beta = 1.0 + (2.0*(yu-y2)/(y2-y1));
                    alpha = 2.0 - pow(beta,-(eta_c+1.0));
                    if (rand <= (1.0/alpha))
                    {
                        betaq = pow ((rand*alpha),(1.0/(eta_c+1.0)));
                    }
                    else
                    {
                        betaq = pow ((1.0/(2.0 - rand*alpha)),(1.0/(eta_c+1.0)));
                    }
                    c2 = 0.5*((y1+y2)+betaq*(y2-y1));
                    if (c1<yl)
                        c1=yl;
                    if (c2<yl)
                        c2=yl;
                    if (c1>yu)
                        c1=yu;
                    if (c2>yu)
                        c2=yu;
                    if (randomperc()<=0.5)
                    {
                        child1->xreal[i] = c2;
                        child2->xreal[i] = c1;
                    }
                    else
                    {
                        child1->xreal[i] = c1;
                        child2->xreal[i] = c2;
                    }
                }
                else
                {
                    child1->xreal[i] = parent1->xreal[i];
                    child2->xreal[i] = parent2->xreal[i];
                }
            }
            else
            {
                child1->xreal[i] = parent1->xreal[i];
                child2->xreal[i] = parent2->xreal[i];
            }
        }
    }
    else
    {
        for (i=0; i<nreal; i++)
        {
            child1->xreal[i] = parent1->xreal[i];
            child2->xreal[i] = parent2->xreal[i];
        }
    }
    return;
}

// /* Routine for two point binary crossover */
// void bincross(individual *parent1, individual *parent2, individual *child1, individual *child2, Instance *instance) {
//     int i, j;
//     double rand;
//     int temp, site1, site2;

//     // Realizar cruce SOLO sobre gene[0] (índices de arcos usados)
//     rand = randomperc();
//     if (rand <= pcross_bin) {
//         nbincross++;
//         site1 = rnd(0, nbits[0] - 1);
//         site2 = rnd(0, nbits[0] - 1);
//         if (site1 > site2) {
//             temp = site1;
//             site1 = site2;
//             site2 = temp;
//         }
//         for (j = 0; j < site1; j++) {
//             child1->gene[0][j] = parent1->gene[0][j];
//             child2->gene[0][j] = parent2->gene[0][j];
//         }
//         for (j = site1; j < site2; j++) {
//             child1->gene[0][j] = parent2->gene[0][j];
//             child2->gene[0][j] = parent1->gene[0][j];
//         }
//         for (j = site2; j < nbits[0]; j++) {
//             child1->gene[0][j] = parent1->gene[0][j];
//             child2->gene[0][j] = parent2->gene[0][j];
//         }
//     } else {
//         for (j = 0; j < nbits[0]; j++) {
//             child1->gene[0][j] = parent1->gene[0][j];
//             child2->gene[0][j] = parent2->gene[0][j];
//         }
//     }

//     //reconstruir gene[1] (arcos usados) y gene[2] (tiempos de llegada)
//     for (j = 0; j < nbits[1]; j++) {
//         child1->gene[1][j] = 0;
//         child2->gene[1][j] = 0;
//     }
//     for (j = 0; j < nbits[0]; j++) {
//         int arc_idx1 = child1->gene[0][j];
//         int arc_idx2 = child2->gene[0][j];
//         if (arc_idx1 >= 0 && arc_idx1 < nbits[1]) child1->gene[1][arc_idx1] = 1;
//         if (arc_idx2 >= 0 && arc_idx2 < nbits[1]) child2->gene[1][arc_idx2] = 1;
//     }


//     // (opcional) Si necesitas cruzar gene[2], repite lo mismo
// }

void bincross(individual *parent1, individual *parent2, individual *child1, individual *child2, Instance *inst) {
    int num_arcs = inst->num_arcs;
    int *common_arcs = (int*)malloc(num_arcs * sizeof(int));
    int num_common = 0;

    // 1. Encontrar todos los índices donde ambos padres tienen 1 en gene[1]
    for (int i = 0; i < nbits[1]; i++) {
        if (parent1->gene[1][i] == 1 && parent2->gene[1][i] == 1) {
            common_arcs[num_common++] = i;
        }
    }

    // 2. Si no hay arcos comunes, copiar directamente los genes
    if (num_common == 0) {
        for (int j = 0; j < nbits[0]; j++) {
            child1->gene[0][j] = parent1->gene[0][j];
            child2->gene[0][j] = parent2->gene[0][j];
        }
    } else {
        // 3. Seleccionar aleatoriamente un punto de cruce (índice de arco común)
        int selected_arc = common_arcs[rnd(0, num_common - 1)];

        // 4. Crear gene[1] de los hijos intercambiando partes
        for (int i = 0; i < nbits[1]; i++) {
            // Copiar hasta el arco seleccionado (inclusive) del padre original
            if (i <= selected_arc) {
                child1->gene[1][i] = parent1->gene[1][i];
                child2->gene[1][i] = parent2->gene[1][i];
            } else {
                // Después del punto de cruce, se intercambian
                child1->gene[1][i] = parent2->gene[1][i];
                child2->gene[1][i] = parent1->gene[1][i];
            }
        }

        // 5. Reconstruir gene[0] a partir de gene[1]
        int idx1 = 0, idx2 = 0;
        for (int i = 0; i < nbits[1]; i++) {
            if (child1->gene[1][i] == 1) child1->gene[0][idx1++] = i;
            if (child2->gene[1][i] == 1) child2->gene[0][idx2++] = i;
        }

        // 6. Rellenar con -1 el resto de gene[0]
        while (idx1 < nbits[0]) child1->gene[0][idx1++] = -1;
        while (idx2 < nbits[0]) child2->gene[0][idx2++] = -1;
    }

    // 7. Generar soluciones completas desde los genes binarios
    //child1->sol = decode_gene_to_solution(child1, inst);
    //child2->sol = decode_gene_to_solution(child2, inst);
    free(common_arcs);
}

