// # ifndef _GLOBAL_H_
// # define _GLOBAL_H_
// #include "model.h"
// #include "GraphReader.h"

// typedef struct
// {
//     int rank;
//     double constr_violation;
//     double crowd_dist;
//     Solution sol;
// } individual;

// typedef struct
// {
//     individual *ind;
// } population;

// void allocate_memory_pop (population *pop, int size);
// void solution_to_individual(Solution *sol, individual *ind, Instance *inst);
// void allocate_memory_ind (individual *ind);


// #endif

/* This file contains the variable and function declarations */

# ifndef _GLOBAL_H_
# define _GLOBAL_H_

# define INF 1.0e14
# define EPS 1.0e-14
# define E  2.71828182845905
# define PI 3.14159265358979
# define GNUPLOT_COMMAND "gnuplot -persist"
#include "model.h"

typedef struct
{
    int rank;
    double constr_violation;
    double *xreal;
    int **gene;
    double *xbin;
    double *obj;
    double *constr;
    double crowd_dist;
    Solution *sol;
}
individual;

typedef struct
{
    individual *ind;
}
population;

typedef struct lists
{
    int index;
    struct lists *parent;
    struct lists *child;
}
list;

typedef struct
{
    int id;
    long int names[3];
}depot;

typedef struct
{
    int id, name, demand;
}client;

typedef struct
{
    int id, capacity;
    int *dStart;
    int *dEnd;
    int *dIn;
    int ndStart, ndEnd, ndIn;
}truck;


typedef struct
{
    int **distances;

    depot *depots;
    client *clients;
    truck *trucks;
    int nDepots, nClients, nTrucks;
}problem_instance;

extern int nreal;
extern int nbin;
extern int nobj;
extern int ncon;
extern int popsize;
extern double seed;
extern double pcross_real;
extern double pcross_bin;
extern double pmut_real;
extern double pmut_bin;
extern double eta_c;
extern double eta_m;
extern int ngen;
extern int nbinmut;
extern int nrealmut;
extern int nbincross;
extern int nrealcross;
extern int *nbits;
extern double *min_realvar;
extern double *max_realvar;
extern double *min_binvar;
extern double *max_binvar;
extern int bitlength;
extern int choice;
extern int obj1;
extern int obj2;
extern int obj3;
extern int angle1;
extern int angle2;

void solution_to_individual(Solution *sol, individual *ind, Instance *inst);
void free_individual(individual *ind);
void allocate_memory_pop (population *pop, int size);
void allocate_memory_ind (individual *ind);
void deallocate_memory_pop (population *pop, int size);
void deallocate_memory_ind (individual *ind);

double maximum (double a, double b);
double minimum (double a, double b);

void crossover (individual *parent1, individual *parent2, individual *child1, individual *child2, Instance *instance);
void route_crossover (individual *parent1, individual *parent2, individual *child1, individual *child2, Instance *instance);
void realcross (individual *parent1, individual *parent2, individual *child1, individual *child2);
void bincross (individual *parent1, individual *parent2, individual *child1, individual *child2, Instance *instance);
Solution* decode_gene_to_solution(individual *ind, Instance *inst);
Solution* copy_solution(Solution *src, Instance *inst);
Solution* create_empty_solution(Instance *inst);

void assign_crowding_distance_list (population *pop, list *lst, int front_size);
void assign_crowding_distance_indices (population *pop, int c1, int c2);
void assign_crowding_distance (population *pop, int *dist, int **obj_array, int front_size);

void decode_pop (population *pop);
void decode_ind (individual *ind);

void onthefly_display (population *pop, FILE *gp, int ii);

int check_dominance (individual *a, individual *b);

void evaluate_pop (population *pop, Instance *inst);
void evaluate_ind (individual *ind, Instance *inst);
void evaluate_solution(individual *ind, Instance *inst);

void fill_nondominated_sort (population *mixed_pop, population *new_pop);
void crowding_fill (population *mixed_pop, population *new_pop, int count, int front_size, list *cur);

void initialize_pop (population *pop);
void initialize_ind (individual *ind);

void insert (list *node, int x);
list* del (list *node);

void merge(population *pop1, population *pop2, population *pop3);
void copy_ind (individual *ind1, individual *ind2);

void mutation_pop (population *pop, Instance *inst);
void mutation_ind (individual *ind, Instance *inst);
void bin_mutate_ind (individual *ind);
void real_mutate_ind (individual *ind);
bool bfs_patch(Instance *inst, int from, int to, int *patch, int *patch_len);
int find_arc_index(Instance *inst, int from, int to);


void test_problem (double *xreal, double *xbin, int **gene, double *obj, double *constr);

void assign_rank_and_crowding_distance (population *new_pop);

void report_pop (population *pop, FILE *fpt);
void report_feasible (population *pop, FILE *fpt);
void report_ind (individual *ind, FILE *fpt);

void quicksort_front_obj(population *pop, int objcount, int obj_array[], int obj_array_size);
void q_sort_front_obj(population *pop, int objcount, int obj_array[], int left, int right);
void quicksort_dist(population *pop, int *dist, int front_size);
void q_sort_dist(population *pop, int *dist, int left, int right);

void selection (population *old_pop, population *new_pop, Instance *inst);
individual* tournament (individual *ind1, individual *ind2);

void evaluate_objective_g(const Instance *instance, individual *ind);
int check_flow_balance_g(const Instance *instance, const individual *ind);
int check_no_subtours_g(const Instance *instance, const individual *ind);
int check_resource_limit_g(const Instance *instance, const individual *ind);
int check_no_forbidden_nodes_g(const Instance *instance, const individual *ind);
int check_required_nodes_g(const Instance *instance, const individual *ind);
int validate_time_windows_g(const individual *ind, const Instance *inst);
bool already_recorded_g(int node, individual *ind, int count);


#endif
