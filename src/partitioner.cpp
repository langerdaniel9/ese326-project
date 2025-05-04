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
            nodeToNets[nodeName].push_back(netName);
            netToNodes[netName].push_back(nodeName);
        }
    }
}

void Partitioner::initializePartition() {
    areaA = areaB = totalArea = 0;
    countA = countB = totalCount = 0;

    for (const auto& [name, node] : nodes) {
        if (node.type == NodeType::Terminal || node.type == NodeType::TerminalNI)
            continue;

        int area = node.width * node.height;
        totalArea += area;
        totalCount++;

        // Partition by area or number of gates, enforcing cap
        if (areaDef == AreaDef::Area) {
            if (areaA + area <= cap) {
                nodeToPartition[name] = "A";
                areaA += area;
            } else {
                nodeToPartition[name] = "B";
                areaB += area;
            }
        } else {  // AreaDef::Num
            if (countA + 1 <= cap) {
                nodeToPartition[name] = "A";
                countA++;
            } else {
                nodeToPartition[name] = "B";
                countB++;
            }
        }
        locked[name] = false;
    }
}

void Partitioner::computeInitialGains() {
    nodeGains.clear();
    gainBucket.clear();

    for (const auto& [nodeName, part] : nodeToPartition) {
        int gain = 0;
        for (const auto& netName : nodeToNets[nodeName]) {
            const auto& nodesInNet = netToNodes[netName];
            int fromCount = 0;
            int toCount = 0;
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
}

void Partitioner::runFM() {
    bool improved = true;
    while (improved) {
        improved = false;
        runOnePass();
        int cut = calculateCutSize();
        std::cout << "Cut size: " << cut << std::endl;
    }
}

void Partitioner::runOnePass() {
    int moved = 0;

    while (!gainBucket.empty()) {
        auto it = gainBucket.rbegin();
        int maxGain = it->first;
        auto& candidates = it->second;

        if (candidates.empty()) {
            gainBucket.erase(maxGain);
            continue;
        }

        std::string bestNode = *candidates.begin();
        candidates.erase(candidates.begin());
        if (candidates.empty()) gainBucket.erase(maxGain);

        if (locked[bestNode]) continue;

        std::string& part = nodeToPartition[bestNode];
        std::string otherPart = (part == "A") ? "B" : "A";
        int area = nodes.at(bestNode).width * nodes.at(bestNode).height;

        // --- Enforce cap for area or number ---
        bool canMove = true;
        if (areaDef == AreaDef::Area) {
            if (otherPart == "A" && areaA + area > cap) canMove = false;
            if (otherPart == "B" && areaB + area > cap) canMove = false;
        } else {  // AreaDef::Num
            if (otherPart == "A" && countA + 1 > cap) canMove = false;
            if (otherPart == "B" && countB + 1 > cap) canMove = false;
        }
        if (!canMove) continue;

        // --- Update partition and stats ---
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
        moved++;

        computeInitialGains();
    }
    for (auto& [k, v] : locked) v = false;
}

int Partitioner::calculateCutSize() const {
    int cut = 0;
    for (const auto& [netName, net] : nets) {
        std::unordered_set<std::string> parts;
        for (const auto& [nodeName, _] : net.pins) {
            if (nodeToPartition.count(nodeName))
                parts.insert(nodeToPartition.at(nodeName));
        }
        if (parts.size() > 1) cut++;
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