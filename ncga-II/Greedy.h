#ifndef GREEDY_H
#define GREEDY_H

#include "Model.h"
#include <random>
#include <set>
#include <string>
#include <vector>
#include <map>

class Greedy {
public:
    struct Solution {
        std::map<std::pair<std::string, std::string>, bool> x; // Arcos seleccionados
        std::map<std::string, double> u; // Orden de visita
        std::map<std::string, double> arrival_time; // Tiempos de llegada
        std::vector<double> F; // Objetivos (F[1] y F[2])
        Solution() : F(3, 0.0) {} // Inicializa F con 3 elementos (índice 0 no usado)
    };

    Greedy(Model& model, unsigned int seed);
    std::vector<Solution> generateSolutions(int n_solutions);
    void saveSolutions(const std::vector<Solution>& solutions, const std::string& filename) const;

private:
    Model& model_;
    std::mt19937 rng_;

    Solution generateSingleSolution();
    bool isValidArc(const std::pair<std::string, std::string>& arc,
                    const std::set<std::string>& visited,
                    double current_time,
                    double current_resource,
                    const std::set<std::string>& required_remaining) const;
    double evaluateArc(const std::pair<std::string, std::string>& arc) const;
};

#endif