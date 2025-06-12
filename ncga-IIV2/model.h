#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <utility>

// Hash para std::pair<string,string>
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);  // desplazamiento para mejor mezcla
    }
};

struct Arc {
    std::string from;
    std::string to;
    double distance;
    double time;
    double resource;
};

class Model {
public:
    std::set<std::string> N; // Nodos
    std::set<std::pair<std::string, std::string>> A; // Arcos
    std::vector<Arc> arcs;

    std::string s; // Nodo origen
    std::string t; // Nodo destino

    std::unordered_map<std::string, double> risk;
    std::unordered_map<std::string, double> et;
    std::unordered_map<std::string, double> lt;
    std::unordered_map<std::string, double> d; // Llegada al nodo
    std::unordered_map<std::string, bool> must_visit;
    std::unordered_map<std::string, bool> prohibited;

    std::unordered_map<std::pair<std::string, std::string>, bool, pair_hash> x;

    double alpha;
    double R;

    Model();
    void initialize();
    bool is_feasible();
    double objective_distance_time();
    double objective_distance_risk();
};

#endif
