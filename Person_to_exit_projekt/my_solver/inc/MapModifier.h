#pragma once
#include "Grid.h"
#include <string>
#include <random>

class MapModifier {
public:
    MapModifier(const std::string& raw_map_content, std::string filename);
    
    // Vrací tuple: {HumanPos, ExitPos, NewFilename, NewContent}
    std::tuple<std::pair<int, int>, std::pair<int, int>, std::string, std::string> generate();

private:
    GridMap grid;
    std::string raw_content;
    std::string input_filename;
    std::mt19937 rng; // Random generator

    std::pair<int, int> find_random_walkable();
    std::pair<int, int> find_random_edge();
};
