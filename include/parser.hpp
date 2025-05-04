#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "net.hpp"
#include "node.hpp"

class Parser {
   public:
    bool loadAuxFile(const std::string& path);
    void printSummary() const;
    void printNets() const;
    const std::unordered_map<std::string, Node>& getNodes() const;
    const std::unordered_map<std::string, Net>& getNets() const;

   private:
    bool loadNodesFile(const std::string& path);
    bool loadNetsFile(const std::string& path);

    std::string nodesFilePath;
    std::string netsFilePath;
    
    std::unordered_map<std::string, Node> nodes;
    std::unordered_map<std::string, Net> nets;
};
