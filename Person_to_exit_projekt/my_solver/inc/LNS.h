#pragma once
#include "Grid.h"
#include <map>
#include <vector>

class LNS {
public:
    LNS(GridMap& grid, std::pair<int, int> human_start, std::pair<int, int> exit_pos);

    // Načte tvůj "test" soubor (cesty robotů)
    void load_paths(const std::map<int, std::vector<std::pair<int, int>>>& paths);

    // Spustí opravování
    void solve(int max_iterations);

    // Vrátí opravené cesty
    std::map<int, std::vector<std::pair<int, int>>> get_paths() const;

private:
    GridMap& grid;
    std::pair<int, int> human_start;
    std::pair<int, int> exit_pos;
    std::map<int, std::vector<std::pair<int, int>>> robot_paths;
    std::vector<std::pair<int, int>> human_ideal_path;

    // Najde IDčka robotů, kteří stojí na cestě člověka v čase t
    std::vector<int> find_blockers(int t);
};
