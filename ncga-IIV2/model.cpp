#include "model.h"
#include <cmath>
#include <algorithm>
#include <iostream>

Model::Model() {
    alpha = 0.5;
    R = 0.0;
}

void Model::initialize() {
    // Aquí puedes cargar nodos, arcos y parámetros desde archivo o definirlos manualmente
}

bool Model::is_feasible() {
    double total_resource = 0.0;
    std::set<std::string> visited;

    for (const auto& arc : arcs) {
        if (x[{arc.from, arc.to}]) {
            total_resource += arc.resource;
            visited.insert(arc.from);
            visited.insert(arc.to);
        }
    }

    if (total_resource > R)
        return false;

    for (const auto& [nodo, required] : must_visit) {
        if (required && visited.find(nodo) == visited.end())
            return false;
    }

    for (const auto& [nodo, forbidden] : prohibited) {
        if (forbidden && visited.find(nodo) != visited.end())
            return false;
    }

    for (const auto& nodo : visited) {
        double arrival_time = d[nodo];
        if (arrival_time < et[nodo] || arrival_time > lt[nodo])
            return false;
    }

    return true;
}

double Model::objective_distance_time() {
    double total_distance = 0.0;
    double total_time = 0.0;

    for (const auto& arc : arcs) {
        if (x[{arc.from, arc.to}]) {
            total_distance += arc.distance;
            total_time += arc.time;
        }
    }

    return alpha * total_distance + (1 - alpha) * total_time;
}

double Model::objective_distance_risk() {
    double total_distance = 0.0;
    double total_risk = 0.0;

    for (const auto& arc : arcs) {
        if (x[{arc.from, arc.to}]) {
            total_distance += arc.distance;
            total_risk += risk[arc.to]; // riesgo del nodo de destino
        }
    }

    return alpha * total_distance + (1 - alpha) * total_risk;
}
