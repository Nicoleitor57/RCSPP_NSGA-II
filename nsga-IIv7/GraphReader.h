#ifndef GRAPHREADER_H
#define GRAPHREADER_H

// Definiciones de estructuras
typedef struct {
    int id;
    float risk;
    int earliest;
    int latest;
} Node;

typedef struct {
    Node from;
    Node to;
    int dist;
    int time;
    int res;
    float epsilon;
} Arc;

typedef struct {
    int source;
    int sink;
    int R_max;
    int *required;
    int num_required;
    int *forbidden;
    int num_forbidden;
    Node *nodes;
    Arc *arcs;
    int num_arcs;
    int **outgoing_arcs; // Arcos salientes por nodo
    int *out_degrees; // Grado de salida por nodo
} Instance;

// Declaración de funciones
int extract_num_nodes(const char *filename);
int extract_node_id(const char *node_str);
void convert_to_instance(const char *filename, Instance *instance);

#endif // GRAPHREADER_H
