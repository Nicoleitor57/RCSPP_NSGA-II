#include "GraphReader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

void GraphReader::readGraph(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "No se pudo abrir el archivo: " << filename << std::endl;
        exit(1);
    }

    std::string line;
    enum Section { NONE, NODES, EPSILON, DIST, TIME, RES, RISK, REQUIRED, FORBIDDEN } section = NONE;

    while (std::getline(infile, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line == "NODES") {
            section = NODES;
            continue;
        } else if (line == "EPSILON") {
            section = EPSILON;
            continue;
        } else if (line == "DIST") {
            section = DIST;
            continue;
        } else if (line == "TIME") {
            section = TIME;
            continue;
        } else if (line == "RES") {
            section = RES;
            continue;
        } else if (line == "RISK") {
            section = RISK;
            continue;
        } else if (line == "REQUIRED") {
            section = REQUIRED;
            continue;
        } else if (line == "FORBIDDEN") {
            section = FORBIDDEN;
            continue;
        }

        std::istringstream iss(line);
        std::string from, to;
        int value;

        switch (section) {
            case NODES:
                nodes.push_back(line);
                break;

            case EPSILON:
                iss >> from >> to;
                break;

            default:
                break;
        }
    }

    // Asignar índices
    nodeToIndex.clear();
    indexToNode.clear();
    int idx = 0;
    for (const std::string& node : nodes) {
        nodeToIndex[node] = idx++;
        indexToNode.push_back(node);
    }

    int n = nodes.size();
    epsilon.assign(n, std::vector<int>(n, 0));
    dist.assign(n, std::vector<int>(n, 0));
    time.assign(n, std::vector<int>(n, 0));
    res.assign(n, std::vector<int>(n, 0));
    risk.assign(n, 0);

    // Releer archivo para llenar parámetros
    infile.clear();
    infile.seekg(0);
    section = NONE;

    while (std::getline(infile, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line == "EPSILON") {
            section = EPSILON;
            continue;
        } else if (line == "DIST") {
            section = DIST;
            continue;
        } else if (line == "TIME") {
            section = TIME;
            continue;
        } else if (line == "RES") {
            section = RES;
            continue;
        } else if (line == "RISK") {
            section = RISK;
            continue;
        } else if (line == "REQUIRED") {
            section = REQUIRED;
            continue;
        } else if (line == "FORBIDDEN") {
            section = FORBIDDEN;
            continue;
        } else if (line == "NODES") {
            section = NODES;
            continue;
        }

        std::istringstream iss(line);
        switch (section) {
            case EPSILON: {
                std::string from, to;
                iss >> from >> to;
                if (nodeToIndex.count(from) && nodeToIndex.count(to)) {
                    epsilon[nodeToIndex[from]][nodeToIndex[to]] = 1;
                } else {
                    std::cerr << "Ignorando arco (" << from << ", " << to << ") en epsilon: nodo no existe." << std::endl;
                }
                break;
            }
            case DIST:
            case TIME:
            case RES: {
                std::string from, to;
                int val;
                iss >> from >> to >> val;
                if (nodeToIndex.count(from) && nodeToIndex.count(to)) {
                    int i = nodeToIndex[from];
                    int j = nodeToIndex[to];
                    if (section == DIST) dist[i][j] = val;
                    else if (section == TIME) time[i][j] = val;
                    else if (section == RES) res[i][j] = val;
                } else {
                    std::cerr << "Ignorando arco (" << from << ", " << to << ") en sección " << section << ": nodo no existe." << std::endl;
                }
                break;
            }
            case RISK: {
                std::string node;
                int r;
                iss >> node >> r;
                if (nodeToIndex.count(node)) {
                    risk[nodeToIndex[node]] = r;
                } else {
                    std::cerr << "Ignorando riesgo para nodo " << node << ": nodo no existe." << std::endl;
                }
                break;
            }
            case REQUIRED: {
                std::string node;
                iss >> node;
                requiredNodes.insert(node);
                break;
            }
            case FORBIDDEN: {
                std::string node;
                iss >> node;
                forbiddenNodes.insert(node);
                break;
            }
            default:
                break;
        }
    }

    // Verificación
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (epsilon[i][j] == 1 && (dist[i][j] == 0 && time[i][j] == 0 && res[i][j] == 0))
                std::cerr << "Arco " << indexToNode[i] << " -> " << indexToNode[j] << " falta en parámetros\n";

    std::cout << "Nodos prohibidos leídos (" << forbiddenNodes.size() << "): ";
    for (const auto& n : forbiddenNodes) std::cout << n << " ";
    std::cout << std::endl;

    std::cout << "Nodos requeridos leídos (" << requiredNodes.size() << "): ";
    for (const auto& n : requiredNodes) std::cout << n << " ";
    std::cout << std::endl;
}
