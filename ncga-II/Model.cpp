#include "Model.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>

Model::Model(const GraphReader& reader, double alpha) : alpha(alpha) {
    initializeFromReader(reader);
    computeRiskMultiplier();
    
    // Inicializar variables
    for (const auto& arc : arcs) {
        x[arc] = false;
    }
    for (const auto& node : nodes) {
        u[node] = 0.0;
        arrival_time[node] = 0.0;
    }
    F[1] = 0.0; // Objetivo 1: distancia + epsilon
    F[2] = 0.0; // Objetivo 2: tiempo + epsilon con riesgo
}

void Model::initializeFromReader(const GraphReader& reader) {
    if (!reader.isValid()) {
        std::cerr << "Error: GraphReader no válido" << std::endl;
        return;
    }
    
    nodes = reader.getNodes();
    forbidden_nodes = reader.getForbiddenNodes();
    required_nodes = reader.getRequiredNodes();
    source = reader.getSource();
    sink = reader.getSink();
    dist = reader.getDist();
    time_ = reader.getTime();
    resource = reader.getResource();
    R_max = reader.getRMax();
    earliest = reader.getEarliest();
    latest = reader.getLatest();
    epsilon = reader.getEpsilon();
    risk_node = reader.getRiskNode();
    
    // Construir conjunto de arcos
    const auto& adjList = reader.getAdjList();
    for (size_t i = 0; i < adjList.size(); ++i) {
        std::string from;
        for (const auto& node : nodes) {
            if (reader.getNodes().count(node) && reader.getNodes().size() > i) {
                auto it = reader.getNodes().begin();
                std::advance(it, i);
                if (it != reader.getNodes().end()) {
                    from = *it;
                    break;
                }
            }
        }
        for (size_t j : adjList[i]) {
            std::string to;
            auto it = reader.getNodes().begin();
            std::advance(it, j);
            if (it != reader.getNodes().end()) {
                to = *it;
                arcs.insert({from, to});
            }
        }
    }
}

void Model::computeRiskMultiplier() {
    for (const auto& arc : arcs) {
        const std::string& i = arc.first;
        const std::string& j = arc.second;
        double risk_i = risk_node.count(i) ? risk_node.at(i) : 0.0;
        double risk_j = risk_node.count(j) ? risk_node.at(j) : 0.0;
        double risk_combined = risk_i + risk_j - risk_i * risk_j;
        risk_multiplier[arc] = (risk_combined >= 1.0) ? 1.5 : 1.0;
    }
}

void Model::setX(const std::string& i, const std::string& j, bool value) {
    x[{i, j}] = value;
}

void Model::setU(const std::string& node, double value) {
    u[node] = value;
}

void Model::setArrivalTime(const std::string& node, double value) {
    arrival_time[node] = value;
}

void Model::setF(int objective, double value) {
    F[objective] = value;
}

double Model::evaluateObjective1() const {
    double sum = 0.0;
    for (const auto& arc : arcs) {
        if (x.at(arc)) {
            double dist_val = dist.count(arc) ? dist.at(arc) : 0.0;
            double epsilon_val = epsilon.count(arc) ? epsilon.at(arc) : 0.0;
            sum += (dist_val + epsilon_val);
        }
    }
    return sum;
}

double Model::evaluateObjective2() const {
    double sum = 0.0;
    for (const auto& arc : arcs) {
        if (x.at(arc)) {
            double time_val = time_.count(arc) ? time_.at(arc) : 0.0;
            double epsilon_val = epsilon.count(arc) ? epsilon.at(arc) : 0.0;
            double multiplier = risk_multiplier.count(arc) ? risk_multiplier.at(arc) : 1.0;
            sum += (time_val + epsilon_val) * multiplier;
        }
    }
    return sum;
}

bool Model::checkFlowBalance() const {
    for (const auto& i : nodes) {
        double outgoing = 0.0;
        double incoming = 0.0;
        for (const auto& arc : arcs) {
            if (arc.first == i) outgoing += x.at(arc);
            if (arc.second == i) incoming += x.at(arc);
        }
        double expected = (i == source) ? 1.0 : (i == sink) ? -1.0 : 0.0;
        if (std::abs(outgoing - incoming - expected) > 1e-6) {
            return false;
        }
    }
    return true;
}

bool Model::checkNoSubtours() const {
    for (const auto& arc : arcs) {
        const std::string& i = arc.first;
        const std::string& j = arc.second;
        if (i != source && j != source && i != j && x.at(arc)) {
            if (u.at(i) - u.at(j) + nodes.size() * x.at(arc) > nodes.size() - 1 + 1e-6) {
                return false;
            }
        }
    }
    return u.at(source) == 0.0;
}

bool Model::checkResourceLimit() const {
    double total_resource = 0.0;
    for (const auto& arc : arcs) {
        if (x.at(arc)) {
            total_resource += resource.count(arc) ? resource.at(arc) : 0.0;
        }
    }
    return total_resource <= R_max;
}

bool Model::checkNoForbidden() const {
    for (const auto& arc : arcs) {
        if (x.at(arc)) {
            if (forbidden_nodes.count(arc.first) || forbidden_nodes.count(arc.second)) {
                return false;
            }
        }
    }
    return true;
}

bool Model::checkMustVisit() const {
    for (const auto& k : required_nodes) {
        double incoming = 0.0;
        for (const auto& arc : arcs) {
            if (arc.second == k && x.at(arc)) {
                incoming += 1.0;
            }
        }
        if (incoming < 1.0 - 1e-6) {
            return false;
        }
    }
    return true;
}

bool Model::checkTimeWindowLower() const {
    for (const auto& i : nodes) {
        if (i != source && arrival_time.at(i) < earliest.at(i) - 1e-6) {
            return false;
        }
    }
    return true;
}

bool Model::checkTimeWindowUpper() const {
    for (const auto& i : nodes) {
        if (i != source && arrival_time.at(i) > latest.at(i) + 1e-6) {
            return false;
        }
    }
    return true;
}

bool Model::checkArrivalPropagation() const {
    const double M = std::accumulate(time_.begin(), time_.end(), 0.0,
        [](double sum, const auto& p) { return sum + p.second; }) +
        std::max_element(latest.begin(), latest.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; })->second;
    
    for (const auto& arc : arcs) {
        const std::string& i = arc.first;
        const std::string& j = arc.second;
        double time_val = time_.count(arc) ? time_.at(arc) : 0.0;
        double multiplier = risk_multiplier.count(arc) ? risk_multiplier.at(arc) : 1.0;
        if (arrival_time.at(j) + M * (1 - x.at(arc)) <
            arrival_time.at(i) + (time_val * multiplier) * x.at(arc) - 1e-6) {
            return false;
        }
    }
    return true;
}

void Model::reset() {
    x.clear();
    u.clear();
    arrival_time.clear();
    F.clear();
}