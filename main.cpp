#include <iostream>
#include <string>

#include "parser.hpp"
#include "partitioner.hpp"

int main() {
    // Stuff to change
    std::string auxFilePath = "benchmarks/example/example.aux"; // Path to the .aux file
    std::string areaDefStr = "area";  // "area" or "num"
    int maxArea = 1000;  // max area per partition
    int maxNum = 3;     // max number of gates per partition

    AreaDef areaDef = AreaDef::Area;
    int cap;

    if (areaDefStr == "num") {
        areaDef = AreaDef::Num;
        cap = maxNum;  // Example: max 3 gates per partition
    } else if (areaDefStr == "area") {
        areaDef = AreaDef::Area;
        cap = maxArea;  // Example: max area 1000 per partition
    } else {
        std::cerr << "Invalid area definition. Use 'num' or 'area'." << std::endl;
        return 1;
    }

    Parser parser;
    if (!parser.loadAuxFile(auxFilePath)) {
        std::cerr << "Failed to load input files." << std::endl;
        return 1;
    }

    Partitioner partitioner(parser.getNodes(), parser.getNets(), areaDef, cap);
    partitioner.runFM();
    partitioner.printResult();

    return 0;
}