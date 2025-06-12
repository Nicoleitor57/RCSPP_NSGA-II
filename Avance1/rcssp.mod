# --- Conjuntos ---
set NODES;                       # Conjunto de nodos
set ARCS within (NODES cross NODES);  # Arcos dirigidos

# --- Parámetros ---
param source symbolic;           # Nodo de origen
param sink symbolic;             # Nodo de destino

param cost{ARCS} > 0;            # Costo de cada arco
param resource{ARCS} > 0;        # Recurso consumido por cada arco
param R_max > 0;                 # Límite total de recurso

# --- Variables ---
var x{(i,j) in ARCS} binary;     # 1 si se elige el arco (i,j), 0 si no

# --- Función objetivo ---
minimize TotalCost:
    sum{(i,j) in ARCS} cost[i,j] * x[i,j];

# --- Restricción de flujo balanceado ---
subject to FlowBalance {i in NODES}:
    sum{(i,j) in ARCS} x[i,j] - sum{(j,i) in ARCS} x[j,i] =
        if i = source then 1
        else if i = sink then -1
        else 0;


# --- Restricción de recurso ---
subject to ResourceLimit:
    sum{(i,j) in ARCS} resource[i,j] * x[i,j] <= R_max;
