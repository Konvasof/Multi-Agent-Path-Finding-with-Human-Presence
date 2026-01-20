#pragma once
#include "Grid.h"
#include <vector>
#include <map>
#include <utility>
#include <optional>

class Replanner {
public:
    // Najde novou cestu pro agenta 'agent_id'
    // Musí se vyhnout:
    // 1. Zdem (grid)
    // 2. Ostatním robotům (current_paths)
    // 3. Člověku (human_path)
    static std::optional<std::vector<std::pair<int, int>>> find_path(
        int agent_id,
        std::pair<int, int> start,
        std::pair<int, int> goal,
        const GridMap& grid,
        const std::map<int, std::vector<std::pair<int, int>>>& current_paths,
        const std::vector<std::pair<int, int>>& human_path, 
        int max_time
    );
};
