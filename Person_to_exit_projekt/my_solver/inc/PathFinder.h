#pragma once
#include "Grid.h"
#include <vector>
#include <set>
#include <utility>
#include <optional> // C++17

class AStarPathFinder {
public:
    static int heuristic(std::pair<int, int> a, std::pair<int, int> b);

    // Vrací std::nullopt pokud cesta neexistuje
    std::optional<std::vector<std::pair<int, int>>> find_path(
        std::pair<int, int> start,
        std::pair<int, int> goal,
        const GridMap& grid_map,
        const std::set<std::pair<int, int>>& dynamic_obstacles
    );
};
