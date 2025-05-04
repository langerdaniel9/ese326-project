#pragma once
#include <string>
#include <vector>

struct Net {
    std::string name;
    std::vector<std::pair<std::string, std::string>> pins;  // (nodeName, direction)

    Net() = default;
    Net(const std::string& n) : name(n) {}
};