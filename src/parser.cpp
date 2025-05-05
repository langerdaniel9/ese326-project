#include "parser.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

bool Parser::loadAuxFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        while (iss >> token) {
            std::string directory = path.substr(0, path.find_last_of("/\\") + 1);
            if (token.find(".nodes") != std::string::npos) {
                nodesFilePath = directory + token;
            }
            if (token.find(".nets") != std::string::npos) {
                netsFilePath = directory + token;
            }
        }
    }

    if (nodesFilePath.empty() || netsFilePath.empty()) return false;
    return loadNodesFile(nodesFilePath) && loadNetsFile(netsFilePath);
}

bool Parser::loadNodesFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        // Skip header or empty lines
        if (line.empty() || 
            line.find("UCLA") != std::string::npos || 
            line.find("NumNodes") != std::string::npos || 
            line.find("NumTerminals") != std::string::npos) {
            continue;
        }

        std::istringstream iss(line);
        std::string name;
        int width, height;
        std::string terminalFlag;

        if (!(iss >> name >> width >> height)) continue;

        NodeType type = NodeType::Regular;

        if (iss >> terminalFlag) {
            if (terminalFlag == "terminal") {
                type = NodeType::Terminal;
            } else if (terminalFlag == "terminal_NI") {
                type = NodeType::TerminalNI;
            }
        }

        nodes[name] = Node(name, width, height, type);
    }

    return true;
}

bool Parser::loadNetsFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    Net currentNet;
    int pinCount = 0;
    bool readingNet = false;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line.find("UCLA") != std::string::npos ||
            line.find("NumNets") != std::string::npos ||
            line.find("NumPins") != std::string::npos ||
            line[0] == '#') {
            continue;  // Skip header/comments
        }

        std::istringstream iss(line);
        std::string word;
        iss >> word;

        if (word == "NetDegree") {
            // Save previous net if needed
            if (readingNet && !currentNet.name.empty()) {
                nets[currentNet.name] = currentNet;
            }
            std::string colon, netName;
            iss >> colon >> pinCount >> netName;
            currentNet = Net(netName);
            readingNet = true;
        } else if (readingNet) {
            std::string nodeName = word;
            std::string direction, colon;
            float x, y;
            iss >> direction >> colon >> x >> y;
            currentNet.pins.emplace_back(nodeName, direction);

            if (--pinCount == 0) {
                nets[currentNet.name] = currentNet;
                readingNet = false;
            }
        }
    }
    // Save the last net if file doesn't end with NetDegree
    if (readingNet && !currentNet.name.empty() && pinCount == 0) {
        nets[currentNet.name] = currentNet;
    }

    return true;
}

void Parser::printSummary() const {
    std::cout << "Parsed " << nodes.size() << " nodes:" << std::endl;
    for (const auto& [name, node] : nodes) {
        std::cout << "  " << name << ": " << node.width << "x" << node.height;

        switch (node.type) {
            case NodeType::Terminal:
                std::cout << " [terminal]";
                break;
            case NodeType::TerminalNI:
                std::cout << " [terminal_NI]";
                break;
            default:
                break;  // NonTerminal - print nothing
        }

        std::cout << std::endl;
    }
}

void Parser::printNets() const {
    std::cout << "Parsed " << nets.size() << " nets:" << std::endl;
    for (const auto& netPair : nets) {
        const Net& net = netPair.second;  // Get the Net object

        // Print the Net name
        std::cout << "Net: " << net.name << "\n";

        // Print each pin associated with the Net
        for (const auto& pin : net.pins) {
            std::cout << "  Pin: " << pin.first << " Direction: " << pin.second << "\n";
        }
        std::cout << "\n";  // Add a blank line between nets for better readability
    }
}

const std::unordered_map<std::string, Node>& Parser::getNodes() const {
    return nodes;
}

const std::unordered_map<std::string, Net>& Parser::getNets() const {
    return nets;
}