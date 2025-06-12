#ifndef MODEL_H
#define MODEL_H

#include "GraphReader.h"
#include <string>
#include <set>
#include <vector>
#include <map>

class Model {
public:
    Model(const GraphReader& reader, double alpha = 0.5);
    
    // Getters para conjuntos
    const std::set<std::string>& getNodes() const { return nodes; }
    const std::set<std::pair<std::string, std::string>>& getArcs() const { return arcs; }
    const std::set<std::string>& getForbiddenNodes() const { return forbidden_nodes; }
    const std::set<std::string>& getRequiredNodes() const { return required_nodes; }
    
    // Getters para parámetros
    const std::string& getSource() const { return source; }
    const std::string& getSink() const { return sink; }
    const std::map<std::pair<std::string, std::string>, double>& getDist() const { return dist; }
    const std::map<std::pair<std::string, std::string>, double>& getTime() const { return time_; }
    const std::map<std::pair<std::string, std::string>, double>& getResource() const { return resource; }
    double getRMax() const { return R_max; }
    const std::map<std::string, double>& getEarliest() const { return earliest; }
    const std::map<std::string, double>& getLatest() const { return latest; }
    double getAlpha() const { return alpha; }
    const std::map<std::pair<std::string, std::string>, double>& getEpsilon() const { return epsilon; }
    const std::map<std::string, double>& getRiskNode() const { return risk_node; }
    const std::map<std::pair<std::string, std::string>, double>& getRiskMultiplier() const { return risk_multiplier; }
    
    // Getters para variables
    const std::map<std::pair<std::string, std::string>, bool>& getX() const { return x; }
    const std::map<std::string, double>& getU() const { return u; }
    const std::map<std::string, double>& getArrivalTime() const { return arrival_time; }
    const std::map<int, double>& getF() const { return F; }
    
    // Métodos para configurar variables (para el solver)
    void setX(const std::string& i, const std::string& j, bool value);
    void setU(const std::string& node, double value);
    void setArrivalTime(const std::string& node, double value);
    void setF(int objective, double value);
    
    // Métodos para evaluar objetivos y restricciones
    double evaluateObjective1() const;
    double evaluateObjective2() const;
    bool checkFlowBalance() const;
    bool checkNoSubtours() const;
    bool checkResourceLimit() const;
    bool checkNoForbidden() const;
    bool checkMustVisit() const;
    bool checkTimeWindowLower() const;
    bool checkTimeWindowUpper() const;
    bool checkArrivalPropagation() const;

    void reset();

private:
    // Conjuntos
    std::set<std::string> nodes;
    std::set<std::pair<std::string, std::string>> arcs;
    std::set<std::string> forbidden_nodes;
    std::set<std::string> required_nodes;
    
    // Parámetros
    std::string source;
    std::string sink;
    std::map<std::pair<std::string, std::string>, double> dist;
    std::map<std::pair<std::string, std::string>, double> time_;
    std::map<std::pair<std::string, std::string>, double> resource;
    double R_max;
    std::map<std::string, double> earliest;
    std::map<std::string, double> latest;
    double alpha;
    std::map<std::pair<std::string, std::string>, double> epsilon;
    std::map<std::string, double> risk_node;
    std::map<std::pair<std::string, std::string>, double> risk_multiplier;
    
    // Variables
    std::map<std::pair<std::string, std::string>, bool> x; // Binaria
    std::map<std::string, double> u; // Orden de visita
    std::map<std::string, double> arrival_time; // Tiempo de llegada
    std::map<int, double> F; // Funciones objetivo (1: distancia, 2: tiempo con riesgo)
    
    // Métodos auxiliares
    void initializeFromReader(const GraphReader& reader);
    void computeRiskMultiplier();
};

#endif