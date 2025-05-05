#include "partitioner.hpp"

Partitioner::Partitioner(const std::unordered_map<std::string, Node>& n,
                         const std::unordered_map<std::string, Net>& e,
                         AreaDef def,
                         int capVal)
    : nodes(n), nets(e), areaDef(def), cap(capVal) {
    buildNetMappings();
    initializePartition();
    computeInitialGains();
}

void Partitioner::buildNetMappings() {
    for (const auto& [netName, net] : nets) {
        for (const auto& [nodeName, _] : net.pins) {
            // Only add mappings for non-terminal nodes or if the node exists
            if (nodes.count(nodeName) &&
                nodes.at(nodeName).type != NodeType::Terminal &&
                nodes.at(nodeName).type != NodeType::TerminalNI) {
                nodeToNets[nodeName].push_back(netName);
            }
            // We still need all nodes in netToNodes for cut calculation
            netToNodes[netName].push_back(nodeName);
        }
    }
}

void Partitioner::initializePartition() {
    areaA = areaB = totalArea = 0;
    countA = countB = totalCount = 0;

    for (const auto& [name, node] : nodes) {
        if (node.type == NodeType::Terminal || node.type == NodeType::TerminalNI) {
            continue;
        }

        int area = node.width * node.height;
        totalArea += area;
        totalCount++;

        // Assign to the smaller partition, while respecting cap constraints
        if (areaDef == AreaDef::Area) {
            // Check which partition has smaller area and if adding to it stays within cap
            if (areaA <= areaB && areaA + area <= cap) {
                nodeToPartition[name] = "A";
                areaA += area;
            } else if (areaB + area <= cap) {
                nodeToPartition[name] = "B";
                areaB += area;
            } else if (areaA + area <= cap) {
                // If B is full but A still has space
                nodeToPartition[name] = "A";
                areaA += area;
            } else {
                // Neither partition can fit this node within cap
                // Assign to the one with more space left
                if (cap - areaA >= cap - areaB) {
                    nodeToPartition[name] = "A";
                    areaA += area;
                } else {
                    nodeToPartition[name] = "B";
                    areaB += area;
                }
            }
        } else {  // AreaDef::Num
            // Check which partition has fewer nodes and if adding to it stays within cap
            if (countA <= countB && countA + 1 <= cap) {
                nodeToPartition[name] = "A";
                countA++;
            } else if (countB + 1 <= cap) {
                nodeToPartition[name] = "B";
                countB++;
            } else if (countA + 1 <= cap) {
                // If B is full but A still has space
                nodeToPartition[name] = "A";
                countA++;
            } else {
                // Neither partition can fit this node within cap
                // Assign to the one with more space left
                if (cap - countA >= cap - countB) {
                    nodeToPartition[name] = "A";
                    countA++;
                } else {
                    nodeToPartition[name] = "B";
                    countB++;
                }
            }
        }
        locked[name] = false;
    }
}

void Partitioner::computeInitialGains() {
    nodeGains.clear();
    gainBucket.clear();

    for (const auto& [nodeName, part] : nodeToPartition) {
        // Skip terminal nodes completely in gain calculation
        if (nodes.count(nodeName) &&
            (nodes.at(nodeName).type == NodeType::Terminal ||
             nodes.at(nodeName).type == NodeType::TerminalNI)) {
            continue;
        }

        int gain = 0;
        for (const auto& netName : nodeToNets[nodeName]) {
            const auto& nodesInNet = netToNodes[netName];
            int fromCount = 0;
            int toCount = 0;
            for (const auto& n : nodesInNet) {
                // Only consider nodes in partitions for gain calculation
                if (nodeToPartition.count(n) > 0) {
                    if (nodeToPartition[n] == part) {
                        fromCount++;
                    } else {
                        toCount++;
                    }
                }
            }
            if (fromCount == 1) gain++;
            if (toCount == 0) gain--;
        }
        nodeGains[nodeName] = gain;
        gainBucket[gain].insert(nodeName);
    }
}

void Partitioner::updateGain(const std::string& nodeName) {
    int oldGain = nodeGains[nodeName];
    gainBucket[oldGain].erase(nodeName);
    if (gainBucket[oldGain].empty()) {
        gainBucket.erase(oldGain);
    }

    int gain = 0;
    std::string part = nodeToPartition[nodeName];
    for (const auto& netName : nodeToNets[nodeName]) {
        const auto& nodesInNet = netToNodes[netName];
        int fromCount = 0, toCount = 0;
        for (const auto& n : nodesInNet) {
            if (nodeToPartition[n] == part)
                fromCount++;
            else
                toCount++;
        }
        if (fromCount == 1) gain++;
        if (toCount == 0) gain--;
    }
    nodeGains[nodeName] = gain;
    gainBucket[gain].insert(nodeName);
}

void Partitioner::runFM() {
    int prevCut = calculateCutSize();
    while (true) {
        runOnePass();
        int newCut = calculateCutSize();
        if (newCut < prevCut) {
            prevCut = newCut;
        } else {
            break;
        }
    }
}

void Partitioner::runOnePass() {
    std::unordered_map<std::string, std::string> originalPartition = nodeToPartition;
    int originalAreaA = areaA;
    int originalAreaB = areaB;
    int originalCountA = countA;
    int originalCountB = countB;

    int bestCutSize = calculateCutSize();
    int currentCutSize = bestCutSize;
    int movesToBest = 0;
    int movesCount = 0;

    std::vector<int> moveGains;
    std::vector<std::string> moveSequence;

    for (auto& [k, v] : locked) {
        v = false;
    }

    while (!gainBucket.empty() && movesCount < nodeToPartition.size()) {
        auto it = gainBucket.rbegin();
        int maxGain = it->first;
        auto& candidates = it->second;

        if (candidates.empty()) {
            gainBucket.erase(maxGain);
            continue;
        }

        std::string bestNode = *candidates.begin();
        candidates.erase(candidates.begin());
        if (candidates.empty()) {
            gainBucket.erase(maxGain);
        }

        if (locked[bestNode]) {
            continue;
        }

        std::string& part = nodeToPartition[bestNode];
        std::string otherPart = (part == "A") ? "B" : "A";
        int area = nodes.at(bestNode).width * nodes.at(bestNode).height;

        // Enforce cap for area or number 
        bool canMove = true;
        if (areaDef == AreaDef::Area) {
            if (otherPart == "A" && areaA + area > cap) {
                canMove = false;
            }
            if (otherPart == "B" && areaB + area > cap) {
                canMove = false;
            }
        } else {  // AreaDef::Num
            if (otherPart == "A" && countA + 1 > cap) {
                canMove = false;
            }
            if (otherPart == "B" && countB + 1 > cap) {
                canMove = false;
            }
        }
        if (!canMove) {
            continue;
        }

        // Update partition and stats 
        part = otherPart;
        if (areaDef == AreaDef::Area) {
            if (part == "A") {
                areaA += area;
                areaB -= area;
            } else {
                areaB += area;
                areaA -= area;
            }
        } else {
            if (part == "A") {
                countA++;
                countB--;
            } else {
                countB++;
                countA--;
            }
        }
        locked[bestNode] = true;

        for (const auto& netName : nodeToNets[bestNode]) {
            for (const auto& neighbor : netToNodes[netName]) {
                if (neighbor != bestNode && !locked[neighbor]) {
                    updateGain(neighbor);
                }
            }
        }

        moveSequence.push_back(bestNode);
        moveGains.push_back(maxGain);
        movesCount++;

        currentCutSize = calculateCutSize();

        if (currentCutSize < bestCutSize) {
            bestCutSize = currentCutSize;
            movesToBest = movesCount;
        }
    }

    if (movesToBest > 0) {
        nodeToPartition = originalPartition;
        areaA = originalAreaA;
        areaB = originalAreaB;
        countA = originalCountA;
        countB = originalCountB;

        for (int i = 0; i < movesToBest; i++) {
            std::string nodeName = moveSequence[i];
            std::string& part = nodeToPartition[nodeName];
            std::string otherPart = (part == "A") ? "B" : "A";
            int area = nodes.at(nodeName).width * nodes.at(nodeName).height;

            part = otherPart;

            if (areaDef == AreaDef::Area) {
                if (part == "A") {
                    areaA += area;
                    areaB -= area;
                } else {
                    areaB += area;
                    areaA -= area;
                }
            } else {  // AreaDef::Num
                if (part == "A") {
                    countA++;
                    countB--;
                } else {
                    countB++;
                    countA--;
                }
            }
        }
    } else {
        nodeToPartition = originalPartition;
        areaA = originalAreaA;
        areaB = originalAreaB;
        countA = originalCountA;
        countB = originalCountB;
    }

    for (auto& [k, v] : locked) {
        v = false;
    }
}

int Partitioner::calculateCutSize() const {
    int cut = 0;
    for (const auto& [netName, net] : nets) {
        std::unordered_set<std::string> parts;
        for (const auto& [nodeName, _] : net.pins) {
            // Only consider nodes that are in a partition (non-terminals)
            if (nodeToPartition.count(nodeName) > 0) {
                parts.insert(nodeToPartition.at(nodeName));
            }
        }
        if (parts.size() > 1) {
            cut++;
        }
    }
    return cut;
}

void Partitioner::printResult() const {
    std::cout << "Partition A:" << std::endl;
    for (const auto& [name, part] : nodeToPartition) {
        if (part == "A") std::cout << "  " << name << std::endl;
    }
    std::cout << "Partition B:" << std::endl;
    for (const auto& [name, part] : nodeToPartition) {
        if (part == "B") std::cout << "  " << name << std::endl;
    }
}

void Partitioner::printResult(std::ostream& os) const {
    os << "Partition A:" << std::endl;
    for (const auto& [name, part] : nodeToPartition) {
        if (part == "A") {
            os << "  " << name << std::endl;
        }
    }
    os << "Partition B:" << std::endl;
    for (const auto& [name, part] : nodeToPartition) {
        if (part == "B") {
            os << "  " << name << std::endl;
        }
    }
}

bool Partitioner::isPartitionFeasible() const {
    // Check if any node is too large for any partition
    for (const auto& [name, node] : nodes) {
        if (node.type == NodeType::Terminal || node.type == NodeType::TerminalNI) {
            continue;
        }
        int area = node.width * node.height;
        if (areaDef == AreaDef::Area && area > cap) {
            return false;
        }
    }

    // Count nodes in each partition
    int countInA = 0, countInB = 0;
    int areaInA = 0, areaInB = 0;

    for (const auto& [name, part] : nodeToPartition) {
        if (nodes.at(name).type == NodeType::Terminal ||
            nodes.at(name).type == NodeType::TerminalNI) {
            continue;
        }

        int area = nodes.at(name).width * nodes.at(name).height;
        if (part == "A") {
            countInA++;
            areaInA += area;
        } else {
            countInB++;
            areaInB += area;
        }
    }

    // Check if partitions exceed caps
    if (areaDef == AreaDef::Area && (areaInA > cap || areaInB > cap)) {
        return false;
    }

    if (areaDef == AreaDef::Num && (countInA > cap || countInB > cap)) {
        return false;
    }

    // Check if all nodes are assigned
    int assigned = countInA + countInB;
    return assigned == totalCount;
}