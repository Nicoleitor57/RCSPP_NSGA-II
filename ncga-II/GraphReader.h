#ifndef GRAPH_READER_H
#define GRAPH_READER_H

#include <string>
#include <vector>
#include <set>
#include <map>

class GraphReader {
public:
    GraphReader(const std::string& filename);
    bool isValid() const { return valid; }
    size_t getNodeCount() const { return nodeCount; }
    const std::set<std::string>& getNodes() const { return nodes; }
    const std::vector<std::vector<int>>& getAdjList() const { return adjList; }
    const std::string& getSource() const { return source; }
    const std::string& getSink() const { return sink; }
    size_t getRMax() const { return R_max; }
    const std::set<std::string>& getForbiddenNodes() const { return forbidden_nodes; }
    const std::set<std::string>& getRequiredNodes() const { return required_nodes; }
    const std::map<std::pair<std::string, std::string>, double>& getDist() const { return dist; }
    const std::map<std::pair<std::string, std::string>, double>& getTime() const { return time_; }
    const std::map<std::pair<std::string, std::string>, double>& getResource() const { return resource; }
    const std::map<std::pair<std::string, std::string>, double>& getEpsilon() const { return epsilon; }
    const std::map<std::string, double>& getRiskNode() const { return risk_node; }
    const std::map<std::string, double>& getEarliest() const { return earliest; }
    const std::map<std::string, double>& getLatest() const { return latest; }

private:
    size_t nodeCount;
    std::set<std::string> nodes;
    std::vector<std::vector<int>> adjList;
    std::string source;
    std::string sink;
    size_t R_max;
    std::set<std::string> forbidden_nodes;
    std::set<std::string> required_nodes;
    std::map<std::pair<std::string, std::string>, double> dist;
    std::map<std::pair<std::string, std::string>, double> time_;
    std::map<std::pair<std::string, std::string>, double> resource;
    std::map<std::pair<std::string, std::string>, double> epsilon;
    std::map<std::string, double> risk_node;
    std::map<std::string, double> earliest;
    std::map<std::string, double> latest;
    bool valid;

    void readFile(const std::string& filename);
    size_t extractNodeCount(const std::string& filename);
    std::string cleanLine(const std::string& line);
    void parseNodes(const std::string& line, std::set<std::string>& nodes);
    void parseArcs(const std::string& line, std::vector<std::pair<std::string, std::string>>& arcs);
    void parseNodeParams(const std::string& line, std::map<std::string, double>& paramMap);
    void parseArcParams(const std::string& line, std::map<std::pair<std::string, std::string>, double>& paramMap);
};

#endif