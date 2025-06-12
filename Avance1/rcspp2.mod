# --- Conjuntos ---
set NODES;
set ARCS within (NODES cross NODES);

# --- Nuevos conjuntos ---
set FORBIDDEN_NODES within NODES;
set REQUIRED_NODES within NODES;

# --- Parámetros ---
param source symbolic;
param sink symbolic;
param dist{ARCS} > 0;
param time{ARCS} > 0;
param resource{ARCS} > 0;
param R_max > 0;
param alpha >= 0, <= 1;  # Peso entre 0 y 1 para balancear distancia y tiempo


# --- Variables ---
var x{(i,j) in ARCS} binary;

# --- Función objetivo 1 ---
#minimize TotalDistance:
    #sum{(i,j) in ARCS} dist[i,j] * x[i,j];

# --- Función objetivo 2 ---
#minimize TotalTime:
    #sum{(i,j) in ARCS} time[i,j] * x[i,j];
    

minimize WeightedObjective:
    alpha * sum{(i,j) in ARCS} dist[i,j] * x[i,j] +
    (1 - alpha) * sum{(i,j) in ARCS} time[i,j] * x[i,j];


# --- Restricción de flujo balanceado (corregida) ---
subject to FlowBalance {i in NODES}:
    sum{(i,j) in ARCS} x[i,j] - sum{(j,i) in ARCS} x[j,i] =
        if i = source then 1
        else if i = sink then -1
        else 0;

# --- Restricción de recurso ---
subject to ResourceLimit:
    sum{(i,j) in ARCS} resource[i,j] * x[i,j] <= R_max;

# --- Restricción 2: Evitar nodos prohibidos ---
subject to NoForbidden:
    sum{(i,j) in ARCS: i in FORBIDDEN_NODES or j in FORBIDDEN_NODES} x[i,j] = 0;

# --- Restricción 3: Pasar por nodos requeridos ---
subject to MustVisit {k in REQUIRED_NODES}:
    sum{(i,k) in ARCS} x[i,k] >= 1;
