/* Crossover routines */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <string.h>

# include "global.h"
# include "rand.h"

/* Function to cross two individuals */
void crossover (individual *parent1, individual *parent2, individual *child1, individual *child2, Instance *instance)
{
    if (nreal!=0)
    {
        //realcross (parent1, parent2, child1, child2);
        //route_crossover(parent1, parent2, child1, child2, instance);
    }
    if (nbin!=0)
    {
        bincross (parent1, parent2, child1, child2, instance);
        //route_crossover(parent1, parent2, child1, child2, instance);
        
    }
    return;
}

void route_crossover(individual *p1, individual *p2, individual *c1, individual *c2, Instance *inst) {
    if (randomperc() <= pcross_real) {
        // Asumiendo que las rutas están en p1->sol->arc_used_index y su longitud en p1->sol->length

        // 1. Cruzamiento por punto intermedio (1 punto)
        int len1 = p1->sol->length;
        int len2 = p2->sol->length;

        int point1 = rnd(1, len1 - 1);
        int point2 = rnd(1, len2 - 1);

        int num_nodes = inst->sink + 1; // Número de nodos, incluyendo fuente y sumidero

        // Reservar nueva memoria para hijos
        Solution *s1 = (Solution *)malloc(sizeof(Solution));
        Solution *s2 = (Solution *)malloc(sizeof(Solution));
        s1->arc_used_index = (int *)malloc(num_nodes * sizeof(int));
        s2->arc_used_index = (int *)malloc(num_nodes * sizeof(int));

        // 2. Crear nuevas rutas por corte y pegado
        int k = 0;
        for (int i = 0; i < point1; i++) s1->arc_used_index[k++] = p1->sol->arc_used_index[i];
        for (int i = point2; i < len2; i++) s1->arc_used_index[k++] = p2->sol->arc_used_index[i];
        s1->length = k;

        k = 0;
        for (int i = 0; i < point2; i++) s2->arc_used_index[k++] = p2->sol->arc_used_index[i];
        for (int i = point1; i < len1; i++) s2->arc_used_index[k++] = p1->sol->arc_used_index[i];
        s2->length = k;

        // 3. Asignar soluciones a hijos
        c1->sol = s1;
        c2->sol = s2;

        // // 4. Con probabilidad pequeña, aplicar parcheo para mejorar viabilidad
        // if (rnd(0, inst->num_arcs) < inst->num_arcs / 100) {
        //     try_mutation(c1, inst);
        // }
        // if (rnd(0, inst->num_arcs) < inst->num_arcs / 100) {
        //     try_mutation(c2, inst);
        // }
    } else {
        // Copia directa
        c1->sol = copy_solution(p1->sol, inst);
        c2->sol = copy_solution(p2->sol, inst);
    }
}

Solution* copy_solution(Solution *src, Instance *inst) {

    int num_nodes = inst->sink + 1; // Número de nodos, incluyendo fuente y sumidero

    Solution *dest = (Solution *)malloc(sizeof(Solution));
    dest->arc_used_index = (int *)malloc(num_nodes * sizeof(int));
    memcpy(dest->arc_used_index, src->arc_used_index, src->length * sizeof(int));
    dest->length = src->length;
    return dest;
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

/* Routine for two point binary crossover */
void bincross (individual *parent1, individual *parent2, individual *child1, individual *child2, Instance *inst)
{
    int i, j;
    double rand;
    int temp, site1, site2;

    int crossed = 0;
    for (i=0; i<nbin; i++)
    {
        rand = randomperc();
        if (rand <= pcross_bin)
        {
            crossed = 1;
            nbincross++;
            site1 = rnd(0,nbits[i]-1);
            site2 = rnd(0,nbits[i]-1);
            if (site1 > site2)
            {
                temp = site1;
                site1 = site2;
                site2 = temp;
            }
            for (j=0; j<site1; j++)
            {
                child1->gene[i][j] = parent1->gene[i][j];
                child2->gene[i][j] = parent2->gene[i][j];
            }
            for (j=site1; j<site2; j++)
            {
                child1->gene[i][j] = parent2->gene[i][j];
                child2->gene[i][j] = parent1->gene[i][j];
            }
            for (j=site2; j<nbits[i]; j++)
            {
                child1->gene[i][j] = parent1->gene[i][j];
                child2->gene[i][j] = parent2->gene[i][j];
            }
        }
        else
        {
            for (j=0; j<nbits[i]; j++)
            {
                child1->gene[i][j] = parent1->gene[i][j];
                child2->gene[i][j] = parent2->gene[i][j];
            }
        }
    }

    // Actualización de la solución
    if (crossed)
    {
        // Desde el gene generado
        child1->sol = decode_gene_to_solution(child1, inst);
        child2->sol = decode_gene_to_solution(child2, inst);
    }
    else
    {
        // Copia directa
        child1->sol = copy_solution(parent1->sol, inst);
        child2->sol = copy_solution(parent2->sol, inst);
    }
    if (child1->sol == NULL || child2->sol == NULL) {
        fprintf(stderr, "Error: No se pudo crear la solución para los hijos\n");
        exit(1);
    }

    return;
}

Solution* decode_gene_to_solution(individual *ind, Instance *inst) {
    Solution *sol = create_empty_solution(inst);
    for (int i = 0; i < inst->num_arcs; i++) {
        int gene_bit = ind->gene[0][i];
        sol->arc_used[i] = gene_bit;
        if (gene_bit) {
            sol->arc_used_index[sol->length++] = i;
        }
    }
    return sol;
}


Solution* create_empty_solution(Instance *inst) {
    Solution *sol = (Solution *) malloc(sizeof(Solution));
    if (sol == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para Solution\n");
        exit(1);
    }

    // Asumimos que inst->num_arcs está correctamente definido
    sol->arc_used = (int *) calloc(inst->num_arcs, sizeof(int));
    sol->arc_used_index = (int *) malloc(inst->num_arcs * sizeof(int));
    sol->length = 0;

    if (sol->arc_used == NULL || sol->arc_used_index == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para arrays de Solution\n");
        exit(1);
    }

    return sol;
}


