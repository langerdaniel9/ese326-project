#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

#include "parser.hpp"
#include "partitioner.hpp"

int main() {
    // Stuff to change
    std::string auxFilePath = "benchmarks/example2/example2.aux";  // Path to the .aux file
    std::string areaDefStr = "num";  // "area" or "num"
    int maxArea = 1000;  // max area per partition
    int maxNum = 500;     // max number of gates per partition

    AreaDef areaDef = AreaDef::Area;
    int cap;

    if (areaDefStr == "num") {
        areaDef = AreaDef::Num;
        cap = maxNum;  
    } else if (areaDefStr == "area") {
        areaDef = AreaDef::Area;
        cap = maxArea;  
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

    // Check if initial partition is possible
    if (!partitioner.isPartitionFeasible()) {
        std::cerr << "Partition cannot be created: constraints cannot be satisfied." << std::endl;
        // Output error to file as well
        std::filesystem::create_directories("results");
        std::string outFile = "results/" + std::filesystem::path(auxFilePath).stem().string() + ".part";
        std::ofstream fout(outFile);
        fout << "Partition cannot be created: constraints cannot be satisfied." << std::endl;
        return 2;
    }

    partitioner.runFM();

    // Output to file
    std::filesystem::create_directories("results");
    std::string outFile = "results/" + std::filesystem::path(auxFilePath).stem().string() + ".part";
    std::ofstream fout(outFile);
    if (!fout) {
        std::cerr << "Could not open output file for writing." << std::endl;
        return 3;
    }
    partitioner.printResult(fout);

    return 0;
}