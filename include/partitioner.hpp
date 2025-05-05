#pragma once

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "net.hpp"
#include "node.hpp"

enum class AreaDef { Area, Num };

class Partitioner {
   public:
    Partitioner(const std::unordered_map<std::string, Node>& nodes,
                const std::unordered_map<std::string, Net>& nets,
                AreaDef areaDef,
                int cap);

    void runFM();
    void printResult() const;
    void printResult(std::ostream& os) const;

    bool isPartitionFeasible() const;

   private:
    const std::unordered_map<std::string, Node>& nodes;
    const std::unordered_map<std::string, Net>& nets;
    AreaDef areaDef;  // Area or Num

    std::unordered_map<std::string, std::string> nodeToPartition;  // "A" or "B"
    std::unordered_map<std::string, std::vector<std::string>> nodeToNets;
    std::unordered_map<std::string, std::vector<std::string>> netToNodes;

    std::unordered_map<std::string, int> nodeGains;
    std::unordered_map<std::string, bool> locked;
    std::map<int, std::unordered_set<std::string>> gainBucket;

    int areaA = 0;
    int areaB = 0;
    int totalArea = 0;
    int countA = 0;
    int countB = 0;
    int totalCount = 0;

    int cap = 0; 

    void initializePartition();
    void buildNetMappings();
    void computeInitialGains();
    void updateGain(const std::string& nodeName);
    void runOnePass();
    int calculateCutSize() const;
};