#ifndef GRAPHREADER_H
#define GRAPHREADER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class GraphReader {
public:
    std::vector<std::string> nodes;
    std::unordered_map<std::string, int> nodeToIndex;
    std::vector<std::string> indexToNode;

    std::vector<std::vector<int>> epsilon;
    std::vector<std::vector<int>> dist;
    std::vector<std::vector<int>> time;
    std::vector<std::vector<int>> res;
    std::vector<int> risk;

    std::unordered_set<std::string> requiredNodes;
    std::unordered_set<std::string> forbiddenNodes;

    void readGraph(const std::string& filename);
};

#endif
