#include "Greedy.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <numeric>

Greedy::Greedy(Model& model, unsigned int seed) : model_(model), rng_(seed) {}

std::vector<Greedy::Solution> Greedy::generateSolutions(int n_solutions) {
    std::vector<Solution> solutions;
    for (int i = 0; i < n_solutions; ++i) {
        Solution sol = generateSingleSolution();
        // Verificar factibilidad
        model_.reset(); // Resetear el modelo
        for (const auto& arc : sol.x) {
            model_.setX(arc.first.first, arc.first.second, arc.second);
        }
        for (const auto& u : sol.u) {
            model_.setU(u.first, u.second);
        }
        for (const auto& at : sol.arrival_time) {
            model_.setArrivalTime(at.first, at.second);
        }
        if (model_.checkFlowBalance() &&
            model_.checkNoSubtours() &&
            model_.checkResourceLimit() &&
            model_.checkNoForbidden() &&
            model_.checkMustVisit() &&
            model_.checkTimeWindowLower() &&
            model_.checkTimeWindowUpper() &&
            model_.checkArrivalPropagation()) {
            sol.F[1] = model_.evaluateObjective1();
            sol.F[2] = model_.evaluateObjective2();
            solutions.push_back(sol);
        }
    }
    return solutions;
}

Greedy::Solution Greedy::generateSingleSolution() {
    Solution sol;
    const auto& arcs = model_.getArcs();
    std::string current_node = model_.getSource();
    std::set<std::string> visited = {current_node};
    double current_time = 0.0;
    double current_resource = 0.0;
    std::set<std::string> required_remaining = model_.getRequiredNodes();
    sol.u[current_node] = 0.0;
    sol.arrival_time[current_node] = 0.0;
    int step = 1;

    while (current_node != model_.getSink() && !arcs.empty()) {
        // Encontrar arcos válidos desde el nodo actual
        std::vector<std::pair<std::string, std::string>> valid_arcs;
        for (const auto& arc : arcs) {
            if (arc.first == current_node &&
                isValidArc(arc, visited, current_time, current_resource, required_remaining)) {
                valid_arcs.push_back(arc);
            }
        }

        if (valid_arcs.empty()) {
            std::cerr << "No valid arcs from node: " << current_node << std::endl; // Depuración
            break; // No se puede continuar
        }

        // Seleccionar el mejor arco (con algo de aleatoriedad)
        std::vector<double> scores;
        for (const auto& arc : valid_arcs) {
            scores.push_back(evaluateArc(arc));
        }
        std::vector<size_t> indices(scores.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(),
                  [&scores](size_t a, size_t b) { return scores[a] < scores[b]; });

        // Elegir uno de los 3 mejores arcos aleatoriamente
        size_t max_choices = std::min<size_t>(3, indices.size());
        std::uniform_int_distribution<size_t> dist(0, max_choices - 1);
        size_t chosen_idx = indices[dist(rng_)];
        auto chosen_arc = valid_arcs[chosen_idx];

        // Actualizar solución
        sol.x[chosen_arc] = true;
        current_node = chosen_arc.second;
        visited.insert(current_node);
        double time_val = model_.getTime().count(chosen_arc) ? model_.getTime().at(chosen_arc) : 0.0;
        double multiplier = model_.getRiskMultiplier().count(chosen_arc) ? model_.getRiskMultiplier().at(chosen_arc) : 1.0;
        current_time += time_val * multiplier;
        current_resource += model_.getResource().count(chosen_arc) ? model_.getResource().at(chosen_arc) : 0.0;
        sol.arrival_time[current_node] = current_time;
        sol.u[current_node] = step++;
        required_remaining.erase(current_node);
    }

    return sol;
}

bool Greedy::isValidArc(const std::pair<std::string, std::string>& arc,
                        const std::set<std::string>& visited,
                        double current_time,
                        double current_resource,
                        const std::set<std::string>& required_remaining) const {
    const auto& forbidden = model_.getForbiddenNodes();
    const auto& earliest = model_.getEarliest();
    const auto& latest = model_.getLatest();
    const auto& time_ = model_.getTime();
    const auto& resource = model_.getResource();
    const auto& risk_multiplier = model_.getRiskMultiplier();

    // Verificar nodos prohibidos
    if (forbidden.count(arc.second)) {
        return false;
    }

    // Evitar ciclos (excepto si es el sink)
    if (visited.count(arc.second) && arc.second != model_.getSink()) {
        return false;
    }

    // Verificar ventanas de tiempo
    double time_val = time_.count(arc) ? time_.at(arc) : 0.0;
    double multiplier = risk_multiplier.count(arc) ? risk_multiplier.at(arc) : 1.0;
    double arrival = current_time + time_val * multiplier;
    if (earliest.count(arc.second)) {
        if (arrival < earliest.at(arc.second) - 1e-6) {
            return false;
        }
    }
    if (latest.count(arc.second)) {
        if (arrival > latest.at(arc.second) + 1e-6) {
            return false;
        }
    }

    // Verificar límite de recursos
    double resource_val = resource.count(arc) ? resource.at(arc) : 0.0;
    if (current_resource + resource_val > model_.getRMax() + 1e-6) {
        return false;
    }

    // Priorizar nodos obligatorios si están cerca de no cumplirse
    if (!required_remaining.empty() && required_remaining.count(arc.second)) {
        return true; // Favorecer nodos obligatorios
    }

    return true;
}

double Greedy::evaluateArc(const std::pair<std::string, std::string>& arc) const {
    if (!model_.getDist().count(arc)) {
        std::cerr << "Dist missing for arc: " << arc.first << " -> " << arc.second << std::endl;
    }
    double dist_val = model_.getDist().count(arc) ? model_.getDist().at(arc) : 0.0;

    if (!model_.getTime().count(arc)) {
        std::cerr << "Time missing for arc: " << arc.first << " -> " << arc.second << std::endl;
    }
    double time_val = model_.getTime().count(arc) ? model_.getTime().at(arc) : 0.0;

    if (!model_.getEpsilon().count(arc)) {
        std::cerr << "Epsilon missing for arc: " << arc.first << " -> " << arc.second << std::endl;
    }
    double epsilon_val = model_.getEpsilon().count(arc) ? model_.getEpsilon().at(arc) : 0.0;

    if (!model_.getRiskMultiplier().count(arc)) {
        std::cerr << "RiskMultiplier missing for arc: " << arc.first << " -> " << arc.second << std::endl;
    }
    double multiplier = model_.getRiskMultiplier().count(arc) ? model_.getRiskMultiplier().at(arc) : 1.0;

    return model_.getAlpha() * (dist_val + epsilon_val) +
           (1.0 - model_.getAlpha()) * (time_val + epsilon_val) * multiplier;
}

void Greedy::saveSolutions(const std::vector<Solution>& solutions, const std::string& filename) const {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Error al abrir archivo: " << filename << std::endl;
        return;
    }

    out << "Solutions: " << solutions.size() << "\n";
    for (size_t i = 0; i < solutions.size(); ++i) {
        const auto& sol = solutions[i];
        out << "\nSolution " << i + 1 << ":\n";
        out << "Objective 1 (distance + epsilon): " << sol.F[1] << "\n";
        out << "Objective 2 (time + risk): " << sol.F[2] << "\n";
        out << "Path:\n";
        for (const auto& arc : sol.x) {
            if (arc.second) {
                out << arc.first.first << " -> " << arc.first.second << "\n";
            }
        }
        out << "Arrival times:\n";
        for (const auto& at : sol.arrival_time) {
            out << at.first << ": " << at.second << "\n";
        }
        out << "U values:\n";
        for (const auto& u : sol.u) {
            out << u.first << ": " << u.second << "\n";
        }
    }
    out.close();
}