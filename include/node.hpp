#pragma once
#include <string>

enum class NodeType {
    Regular,
    Terminal,
    TerminalNI
};

struct Node {
    std::string name;
    int width;
    int height;
    NodeType type;

    Node() : name(""), width(0), height(0), type(NodeType::Regular) {}
    Node(const std::string& n, int w, int h, NodeType t)
        : name(n), width(w), height(h), type(t) {}

    bool isMovable() const {
        return type == NodeType::Regular || type == NodeType::Terminal;
    }
};