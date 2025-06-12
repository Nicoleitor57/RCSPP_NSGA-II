#include "GraphReader.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <iostream>

GraphReader::GraphReader(const std::string& filename) : nodeCount(0), R_max(0), valid(false) {
    readFile(filename);
}

size_t GraphReader::extractNodeCount(const std::string& filename) {
    std::regex rgx("(\\d+)n\\.txt");
    std::smatch match;
    if (std::regex_search(filename, match, rgx)) {
        return std::stoul(match[1]);
    }
    return 0;
}

std::string GraphReader::cleanLine(const std::string& line) {
    std::string cleaned = line;
    cleaned.erase(std::remove(cleaned.begin(), cleaned.end(), ';'), cleaned.end());
    return cleaned;
}

void GraphReader::parseNodes(const std::string& line, std::set<std::string>& nodes) {
    std::stringstream ss(line);
    std::string node;
    while (ss >> node) {
        // Ignorar tokens vacíos o irrelevantes
        if (!node.empty() && node != ":=") {
            nodes.insert(node);
            //std::cerr << "Nodo parseado: " << node << std::endl; // Depuración
        }
    }
}

void GraphReader::parseArcs(const std::string& line, std::vector<std::pair<std::string, std::string>>& arcs) {
    std::string cleaned = cleanLine(line);
    std::regex arc_regex("\\((\\w+),(\\w+)\\)");
    std::sregex_iterator it(cleaned.begin(), cleaned.end(), arc_regex);
    std::sregex_iterator end;
    while (it != end) {
        arcs.emplace_back(it->str(1), it->str(2));
        ++it;
    }
}

void GraphReader::parseNodeParams(const std::string& line, std::map<std::string, double>& paramMap) {
    std::stringstream ss(cleanLine(line));
    std::string node;
    double value;
    if (ss >> node >> value) {
        paramMap[node] = value;
    }
}

void GraphReader::parseArcParams(const std::string& line, std::map<std::pair<std::string, std::string>, double>& paramMap) {
    std::stringstream ss(cleanLine(line));
    std::string node1, node2;
    double value;
    if (ss >> node1 >> node2 >> value) {
        paramMap[{node1, node2}] = value;
    }
}

void GraphReader::readFile(const std::string& filename) {
    nodeCount = extractNodeCount(filename);
    if (nodeCount == 0) {
        std::cerr << "Nombre de archivo inválido. Debe ser <número>n.txt" << std::endl;
        return;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo: " << filename << std::endl;
        return;
    }

    std::vector<std::pair<std::string, std::string>> arcs;
    std::string line;
    std::string currentSection;
    bool inSigma = false;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line.find("param sigma") != std::string::npos) {
            inSigma = true;
            std::cerr << "Detectada sección sigma" << std::endl;
            continue;
        }
        if (inSigma && line.find(";") != std::string::npos) {
            inSigma = false;
            continue;
        }
        if (inSigma) continue;

        if (line.find("set NODES") != std::string::npos) {
            currentSection = "NODES";
            std::cerr << "Detectada sección NODES" << std::endl;
            continue;
        } else if (line.find("set ARCS") != std::string::npos) {
            currentSection = "ARCS";
            std::cerr << "Detectada sección ARCS" << std::endl;
            continue;
        } else if (line.find("param source") != std::string::npos) {
            std::stringstream ss(line);
            std::string temp;
            ss >> temp >> temp >> temp >> source;
            std::cerr << "Detectado source: " << source << std::endl;
            continue;
        } else if (line.find("param sink") != std::string::npos) {
            std::stringstream ss(line);
            std::string temp;
            ss >> temp >> temp >> temp >> sink;
            std::cerr << "Detectado sink: " << sink << std::endl;
            continue;
        } else if (line.find("param R_max") != std::string::npos) {
            std::stringstream ss(line);
            std::string temp;
            ss >> temp >> temp >> temp >> R_max;
            std::cerr << "Detectado R_max: " << R_max << std::endl;
            continue;
        } else if (line.find("set FORBIDDEN_NODES") != std::string::npos) {
            currentSection = "FORBIDDEN_NODES";
            std::cerr << "Detectada sección FORBIDDEN_NODES" << std::endl;
            // Procesar la línea que contiene los nodos
            std::string node_line = cleanLine(line);
            size_t pos = node_line.find(":=");
            if (pos != std::string::npos) {
                node_line = node_line.substr(pos + 2);
                // node_line.erase(std::remove_if(node_line.begin(), node_line.end(), ::isspace), node_line.end());
                parseNodes(node_line, forbidden_nodes);
            }
            continue;
        } else if (line.find("set REQUIRED_NODES") != std::string::npos) {
            currentSection = "REQUIRED_NODES";
            std::cerr << "Detectada sección REQUIRED_NODES" << std::endl;
            // Procesar la línea que contiene los nodos
            std::string node_line = cleanLine(line);
            size_t pos = node_line.find(":=");
            if (pos != std::string::npos) {
                node_line = node_line.substr(pos + 2);
                //node_line.erase(std::remove_if(node_line.begin(), node_line.end(), ::isspace), node_line.end());
                parseNodes(node_line, required_nodes);
            }
            continue;
        } else if (line.find("param dist") != std::string::npos) {
            currentSection = "DIST";
            std::cerr << "Detectada sección DIST" << std::endl;
            continue;
        } else if (line.find("param time_") != std::string::npos) {
            currentSection = "TIME";
            std::cerr << "Detectada sección TIME" << std::endl;
            continue;
        } else if (line.find("param resource") != std::string::npos) {
            currentSection = "RESOURCE";
            std::cerr << "Detectada sección RESOURCE" << std::endl;
            continue;
        } else if (line.find("param epsilon") != std::string::npos) {
            currentSection = "EPSILON";
            std::cerr << "Detectada sección EPSILON" << std::endl;
            continue;
        } else if (line.find("param risk_node") != std::string::npos) {
            currentSection = "RISK_NODE";
            std::cerr << "Detectada sección RISK_NODE" << std::endl;
            continue;
        } else if (line.find("param earliest") != std::string::npos) {
            currentSection = "EARLIEST";
            std::cerr << "Detectada sección EARLIEST" << std::endl;
            continue;
        } else if (line.find("param latest") != std::string::npos) {
            currentSection = "LATEST";
            std::cerr << "Detectada sección LATEST" << std::endl;
            continue;
        }

        if (currentSection == "NODES") {
            parseNodes(line, nodes);
        } else if (currentSection == "ARCS") {
            parseArcs(line, arcs);
        } else if (currentSection == "DIST") {
            if (line.find("[*,*]") == std::string::npos) {
                parseArcParams(line, dist);
            }
        } else if (currentSection == "TIME") {
            if (line.find("[*,*]") == std::string::npos) {
                parseArcParams(line, time_);
            }
        } else if (currentSection == "RESOURCE") {
            if (line.find("[*,*]") == std::string::npos) {
                parseArcParams(line, resource);
            }
        } else if (currentSection == "EPSILON") {
            if (line.find("[*,*]") == std::string::npos) {
                std::stringstream ss(line);
                std::string node1, node2;
                double value;
                if (ss >> node1 >> node2 >> value) {
                    if (nodes.count(node1) == 0 || nodes.count(node2) == 0) {
                        std::cerr << "Ignorando arco (" << node1 << ", " << node2 << ") en epsilon: nodo no existe." << std::endl;
                        continue;
                    }
                    epsilon[{node1, node2}] = value;
                }
            }
        } else if (currentSection == "RISK_NODE") {
            parseNodeParams(line, risk_node);
        } else if (currentSection == "EARLIEST") {
            parseNodeParams(line, earliest);
        } else if (currentSection == "LATEST") {
            parseNodeParams(line, latest);
        }
    }

    file.close();

    if (nodes.empty()) {
        std::cerr << "No se encontraron nodos en el archivo." << std::endl;
        return;
    }

    // Validar que todos los arcos tengan entradas en los mapas
    for (const auto& arc : arcs) {
        if (!dist.count(arc)) {
            std::cerr << "Arco " << arc.first << " -> " << arc.second << " falta en dist" << std::endl;
        }
        if (!time_.count(arc)) {
            std::cerr << "Arco " << arc.first << " -> " << arc.second << " falta en time_" << std::endl;
        }
        if (!resource.count(arc)) {
            std::cerr << "Arco " << arc.first << " -> " << arc.second << " falta en resource" << std::endl;
        }
        if (!epsilon.count(arc)) {
            std::cerr << "Arco " << arc.first << " -> " << arc.second << " falta en epsilon" << std::endl;
        }
    }

    // Depuración de nodos prohibidos y requeridos
    std::cerr << "Nodos prohibidos leídos (" << forbidden_nodes.size() << "): ";
    for (const auto& node : forbidden_nodes) {
        std::cerr << node << " ";
    }
    std::cerr << std::endl;
    std::cerr << "Nodos requeridos leídos (" << required_nodes.size() << "): ";
    for (const auto& node : required_nodes) {
        std::cerr << node << " ";
    }
    std::cerr << std::endl;

    std::map<std::string, size_t> nodeIndex;
    size_t index = 0;
    for (const auto& node : nodes) {
        nodeIndex[node] = index++;
    }

    adjList.resize(nodeCount);
    for (const auto& arc : arcs) {
        size_t from = nodeIndex[arc.first];
        size_t to = nodeIndex[arc.second];
        adjList[from].push_back(to);
    }

    valid = true;
}