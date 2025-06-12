#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG_PRINTS 0
#if DEBUG_PRINTS
#define DPRINT(...) printf(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

#include "GraphReader.h"

int extract_num_nodes(const char *filepath) {
    // Buscar la última barra '/'
    const char *basename = strrchr(filepath, '/');
    if (basename)
        basename++; // apunta al carácter justo después de '/'
    else
        basename = filepath; // no hay '/', usar todo el string

    // Ahora buscar la 'n' en basename
    const char *p = strchr(basename, 'n');
    if (!p) {
        return -1;
    }

    // Resto igual, solo que en basename
    char num_str[20];
    const char *q = p - 1;
    while (q >= basename && isdigit(*q)) {
        q--;
    }

    int start = (int)(q - basename) + 1;
    int end = (int)(p - basename) - 1;
    int num_len = end - start + 1;

    if (num_len <= 0 || num_len >= (int)sizeof(num_str)) {
        return -1;
    }

    strncpy(num_str, basename + start, num_len);
    num_str[num_len] = '\0';

    return atoi(num_str);
}


int extract_node_id(const char *node_str) {
    if (!node_str || node_str[0] != 'n') {
        return -1;  // formato inválido
    }

    // Asegurar que lo que sigue a la 'n' son dígitos
    const char *p = node_str + 1;
    if (!isdigit(*p)) {
        return -1;
    }

    return atoi(p);
}

// typedef struct{
//     int id;
//     float risk;
//     int earliest;
//     int latest;
// } Node;

// typedef struct {
//     Node from;
//     Node to;
//     int dist;
//     int time;
//     int res;
//     int epsilon;
// } Arc;

// typedef struct {
//     int source;
//     int sink;
//     int R_max;
//     int *required;
//     int num_required;
//     int *forbidden;
//     int num_forbidden;
//     Node *nodes;
//     Arc *arcs;
// } Instance;



void convert_to_instance(const char *filename, Instance *instance) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error al abrir el archivo");
        return;
    }

    // Leer el número de nodos
    int num_nodes = extract_num_nodes(filename);
    if (num_nodes <= 0) {
        fprintf(stderr, "Número de nodos inválido\n");
        fclose(file);
        return;
    }

    // Asignar memoria para los nodos y arcos
    instance->nodes = (Node *)malloc((num_nodes) * sizeof(Node));
    instance->arcs = (Arc *)malloc(num_nodes * 7 * sizeof(Arc));
    instance->out_degrees = calloc(num_nodes * 7 , sizeof(int));
    instance->num_arcs = num_nodes * 7;
    int arc_count = 0;
    



    char line[512];
    while (fgets(line, sizeof(line), file) != NULL) {

        // Buscar source
        if (strstr(line, "param source")) {
            char node[20];
            if (sscanf(line, "param source := %19[^;];", node) == 1) {
                instance->source = extract_node_id(node) -1;
                DPRINT("Source node: %s -> ID: %d\n", node, instance->source);
            }
        }
        // Buscar sink
        if (strstr(line, "param sink")) {
            char node[20];
            if (sscanf(line, "param sink := %19[^;];", node) == 1) {
                instance->sink = extract_node_id(node) -1;
                DPRINT("Sink node: %s -> ID: %d\n", node, instance->sink);
            }
        }

        // Buscar R_max
        else if (strstr(line, "param R_max")) {
            if (sscanf(line, "param R_max := %d;", &instance->R_max) == 1) {
                DPRINT("R_max: %d\n", instance->R_max);
            }
        }

        else if (strstr(line, "set FORBIDDEN_NODES")) {
            char *p = strstr(line, ":=");
            if (p) {
                p += 2; // saltar ":="
                char *token = strtok(p, " ;\n");
                while (token) {
                    if (token[0] == 'n') {
                        int id = extract_node_id(token) - 1;
                        instance->forbidden = realloc(instance->forbidden, (instance->num_forbidden + 1) * sizeof(int));
                        instance->forbidden[instance->num_forbidden++] = id;
                        DPRINT("Forbidden node: %s -> ID: %d\n", token, id);
                    }
                    token = strtok(NULL, " ;\n");
                }
            }
        }
        else if (strstr(line, "set REQUIRED_NODES")) {
            char *p = strstr(line, ":=");
            if (p) {
                p += 2; // saltar ":="
                char *token = strtok(p, " ;\n");
                while (token) {
                    if (token[0] == 'n') {
                        int id = extract_node_id(token) - 1;
                        instance->required = realloc(instance->required, (instance->num_required + 1) * sizeof(int));
                        instance->required[instance->num_required++] = id;
                        DPRINT("Required node: %s -> ID: %d\n", token, id);
                    }
                    token = strtok(NULL, " ;\n");
                }
            }
        }
        else if (strstr(line, "set ARCS :=")) {
            char *p = strstr(line, ":=");

            if (p) {
                while(fgets(line, sizeof(line), file) != NULL) { 
                    
                    if (strchr(line, ';')) {
                        break; // Fin de la sección de arcos
                    }
                    char *token = strtok(line, " \t\n");
                    
                    while(token) {
                        if (token[0] == '(') {
                            char *comma = strchr(token, ',');
                            if (comma) {
                                *comma = '\0'; // separa el nodo origen
                                const char *from_str = token + 1;
                                const char *to_str = comma + 1;

                                // elimina el ')' si está al final
                                char *paren = strchr(to_str, ')');
                                if (paren) *paren = '\0';

                                int from_id = extract_node_id(from_str) - 1;
                                int to_id = extract_node_id(to_str) - 1;

                                instance->arcs[arc_count].from.id = from_id;
                                instance->arcs[arc_count].to.id = to_id;
                                arc_count++;

                                instance->out_degrees[from_id]++;

                                DPRINT("Arco: (%d -> %d)\n", from_id, to_id);
                            }
                        }
                        token = strtok(NULL, " \t\n");

                    }
                }
            }


            instance->outgoing_arcs = malloc(num_nodes * sizeof(int *));
            for (int i = 0; i < num_nodes; i++) {
                instance->outgoing_arcs[i] = malloc(instance->out_degrees[i] * sizeof(int));
                instance->out_degrees[i] = 0; // reseteamos para reutilizar como índice temporal
            }

            instance->arcs = realloc(instance->arcs, arc_count * sizeof(Arc));
            
            instance->num_arcs = arc_count;
            DPRINT("Número de arcos: %d\n", arc_count);
            DPRINT("Numero de nodos: %d\n", num_nodes);
            DPRINT("Número de arcos en instance: %d\n", instance->num_arcs);

            for (int i = 0; i < instance->num_arcs; i++) {
                int from = instance->arcs[i].from.id;
                int pos = instance->out_degrees[from]++;
                instance->outgoing_arcs[from][pos] = i;
            }

        }
        else if (strstr(line, "param dist :=")) {
            // Ignorar la línea siguiente ([*,*])
            fgets(line, sizeof(line), file);

            int arc_index = 0;

            while (fgets(line, sizeof(line), file)) {
                if (strchr(line, ';')) break; // Fin de sección

                char from[20], to[20];
                int dist;

                if (sscanf(line, "%19s %19s %d", from, to, &dist) == 3) {
                    if (arc_index >= arc_count) {
                        fprintf(stderr, "Advertencia: más distancias que arcos\n");
                        break;
                    }
                    instance->arcs[arc_index].dist = dist;
                    DPRINT("Distancia (%s -> %s): %d\n", from, to, dist);
                    arc_index++;
                }
            }
        }
        else if (strstr(line, "param time_ :=")) {
        // Ignorar línea [*,*]
            fgets(line, sizeof(line), file);

            int arc_index = 0;

            while (fgets(line, sizeof(line), file)) {
                if (strchr(line, ';')) break;

                char from[20], to[20];
                int time;

                if (sscanf(line, "%19s %19s %d", from, to, &time) == 3) {
                    if (arc_index >= arc_count) {
                        fprintf(stderr, "Advertencia: más tiempos que arcos\n");
                        break;
                    }
                    instance->arcs[arc_index].time = time;
                    DPRINT("Tiempo (%s -> %s): %d\n", from, to, time);
                    arc_index++;
                }
            }
        }

        else if (strstr(line, "param resource :=")) {
            // Ignorar línea [*,*]
            fgets(line, sizeof(line), file);

            int arc_index = 0;

            while (fgets(line, sizeof(line), file)) {
                if (strchr(line, ';')) break;

                char from[20], to[20];
                int res;

                if (sscanf(line, "%19s %19s %d", from, to, &res) == 3) {
                    if (arc_index >= arc_count) {
                        fprintf(stderr, "Advertencia: más recursos que arcos\n");
                        break;
                    }
                    instance->arcs[arc_index].res = res;
                    DPRINT("Resource (%s -> %s): %d\n", from, to, res);
                    arc_index++;
                }
            }
        }

        else if (strstr(line, "param epsilon :=")) {
            // Ignorar línea [*,*]
            fgets(line, sizeof(line), file);

            int arc_index = 0;

            while (fgets(line, sizeof(line), file)) {
                if (strchr(line, ';')) break;

                char from[20], to[20];
                float epsilon;

                if (sscanf(line, "%19s %19s %f", from, to, &epsilon) == 3) {
                    if (arc_index >= arc_count) {
                        fprintf(stderr, "Advertencia: más epsilon que arcos\n");
                        break;
                    }
                    instance->arcs[arc_index].epsilon = epsilon;
                    DPRINT("Epsilon (%s -> %s): %f\n", from, to, epsilon);
                    arc_index++;
                }
            }
        }
        else if (strstr(line, "param risk_node :=")) {
            while (fgets(line, sizeof(line), file)) {
                if (strchr(line, ';')) break;  // fin de sección

                char node_str[20];
                float risk;

                if (sscanf(line, "%19s %f", node_str, &risk) == 2) {
                    int node_id = extract_node_id(node_str) - 1;

                    if (node_id >= 0 && node_id < num_nodes) {
                        instance->nodes[node_id].id = node_id;
                        instance->nodes[node_id].risk = risk;
                        DPRINT("Node risk: %s -> ID: %d -> Risk: %.2f\n", node_str, node_id, risk);
                    } else {
                        fprintf(stderr, "ID de nodo fuera de rango: %s\n", node_str);
                    }
                }
            }
        }
        else if (strstr(line, "param earliest :=")) {
            while (fgets(line, sizeof(line), file)) {
                if (strchr(line, ';')) break;  // fin de sección

                char node_str[20];
                int earliest;

                if (sscanf(line, "%19s %d", node_str, &earliest) == 2) {
                    int node_id = extract_node_id(node_str) -1;

                    if (node_id >= 0 && node_id < num_nodes) {
                        instance->nodes[node_id].earliest = earliest;
                        DPRINT("Earliest for node %s (ID %d): %d\n", node_str, node_id, earliest);
                    } else {
                        fprintf(stderr, "ID de nodo fuera de rango en earliest: %s\n", node_str);
                    }
                }
            }
        }
        else if (strstr(line, "param latest :=")) {
            while (fgets(line, sizeof(line), file)) {
                if (strchr(line, ';')) break;  // fin de sección

                char node_str[20];
                int latest;

                if (sscanf(line, "%19s %d", node_str, &latest) == 2) {
                    int node_id = extract_node_id(node_str) - 1;

                    if (node_id >= 0 && node_id < num_nodes) {
                        instance->nodes[node_id].latest = latest;
                        DPRINT("Latest for node %s (ID %d): %d\n", node_str, node_id, latest);
                    } else {
                        fprintf(stderr, "ID de nodo fuera de rango en latest: %s\n", node_str);
                    }
                }
            }
        }


    }
    fclose(file);
}



