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
param time_{ARCS} > 0;
param resource{ARCS} > 0;
param R_max > 0;
param earliest{NODES} >= 0;
param latest{NODES} >= 0;
param alpha >= 0, <= 1;

param epsilon{ARCS} >=0;

param risk_node{NODES} >= 0, <= 1;  

# Variable auxiliar para orden de visita (MTZ)
var u{NODES} >= 0, <= card(NODES);


#Parámetro de riesgo: 1 si el nodo es riesgoso, 0 si no
#param risk{NODES} binary;


/* Conjuntos, Parametros y Variables para la normalización */
param cantobj := 2 ; # cantidad de objetivos del problema

param cantejc := 21 ; # cantidad de ejecuciones para la frontera de pareto
set objetivos := {1..cantobj} ; # conjunto de objetivos del problema
set ejecuciones := {1..cantejc} ; # conjunto de ejecuciones para la frontera de pareto
param g default 0 ; # identifica un objetivo en particular
param sigma{ejecuciones,objetivos} ; # ponderadores para la frontera de pareto
param betha{objetivos} default 0 ; # ponderadores de cada objetivo
param MV{objetivos} default 999999999 ; # mejor valor alcanzado por cada objetivo
param PV{objetivos} default 0 ; # peor valor alcanzado por cada objetivo
var F{objetivos} >= 0 ; # funciones objetivos del problema


# --- Variables ---
var x{(i,j) in ARCS} binary;

# Tiempo acumulado de llegada a cada nodo
var arrival_time{NODES} >= 0;

param risk_multiplier{i in NODES, j in NODES} :=
    if (risk_node[i] + risk_node[j] - risk_node[i]*risk_node[j]) >= 1 then 1.5 else 1.0;


/* Funcion objetivo */
minimize FO1 : F[g] ; # para minimizar cada objetivo por separado
minimize FO2 :
    sum {i in objetivos} betha[i] * (MV[i] - F[i]) / max(1e-6, MV[i] - PV[i]);
# para minimizar la funciones objetivos normalizadas

O1 : F[1] = sum{(i,j) in ARCS} (dist[i,j] + epsilon[i,j]) * x[i,j] ;
O2 : F[2] = sum{(i,j) in ARCS} (time_[i,j] + epsilon[i,j]) * risk_multiplier[i,j] * x[i,j] ;


# --- Restricción de flujo balanceado ---
subject to FlowBalance {i in NODES}:
    sum{(i,j) in ARCS} x[i,j] - sum{(j,i) in ARCS} x[j,i] =
        if i = source then 1
        else if i = sink then -1
        else 0;
        
# Fijar orden del nodo source
subject to FixSourceOrder:
    u[source] = 0;

# Evitar subtours (ciclos internos)
subject to NoSubtours {(i,j) in ARCS: i != source and j != source and i != j}:
    u[i] - u[j] + card(NODES) * x[i,j] <= card(NODES) - 1;


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
#subject to ArrivalPropagation {(i,j) in ARCS}:
#    arrival_time[j] >= arrival_time[i] + time_[i,j] * risk_multiplier[i,j] * x[i,j];

