# --- Conjuntos ---
set NODES;
set ARCS within (NODES cross NODES);

# --- Conjuntos adicionales ---
set FORBIDDEN_NODES within NODES;
set REQUIRED_NODES within NODES;

# --- Parámetros ---
param source symbolic;
param sink symbolic;

param dist{ARCS} > 0;
param time{ARCS} > 0;
param resource{ARCS} > 0;
param R_max > 0;
param earliest{NODES} >= 0;
param latest{NODES} >= 0;
param alpha >= 0, <= 1;
param dist_max > 0;
param time_max > 0;


param risk_node{NODES} >= 0, <= 1;  
    

# Parámetro de riesgo: 1 si el nodo es riesgoso, 0 si no
#param risk{NODES} binary;

# --- Variables ---
var x{(i,j) in ARCS} binary;

# Tiempo acumulado de llegada a cada nodo
var arrival_time{NODES} >= 0;

# --- Función objetivo ponderada ---
#param alpha_time := 0.7;
#param alpha_distance := 0.3;

param risk_multiplier{i in NODES, j in NODES} :=
    if (risk_node[i] + risk_node[j] - risk_node[i]*risk_node[j]) >= 1 then 1.5 else 1.0;

# Luego úsalo en el cálculo de tiempo y distancia
minimize ObjectiveFunction:
    alpha * (sum{(i,j) in ARCS} dist[i,j] * x[i,j]) / dist_max
  + (1 - alpha) * (sum{(i,j) in ARCS} time[i,j] * risk_multiplier[i,j] * x[i,j]) / time_max;


# --- Restricción de flujo balanceado ---
subject to FlowBalance {i in NODES}:
    sum{(i,j) in ARCS} x[i,j] - sum{(j,i) in ARCS} x[j,i] =
        if i = source then 1
        else if i = sink then -1
        else 0;

# --- Restricción de recurso total ---
subject to ResourceLimit:
    sum{(i,j) in ARCS} resource[i,j] * x[i,j] <= R_max;

# --- No pasar por nodos prohibidos ---
subject to NoForbidden:
    sum{(i,j) in ARCS: i in FORBIDDEN_NODES or j in FORBIDDEN_NODES} x[i,j] = 0;

# --- Pasar por nodos requeridos ---
subject to MustVisit {k in REQUIRED_NODES}:
    sum{(i,k) in ARCS} x[i,k] >= 1;

# --- Ventanas de tiempo ---
subject to TimeWindowLower {i in NODES: i != source}:
    arrival_time[i] >= earliest[i];

subject to TimeWindowUpper {i in NODES: i != source}:
    arrival_time[i] <= latest[i];


# --- Tiempo acumulado respetando arcos y riesgo ---
subject to ArrivalPropagation {(i,j) in ARCS}:
    arrival_time[j] >= arrival_time[i] + time[i,j] * risk_multiplier[i,j] * x[i,j];




