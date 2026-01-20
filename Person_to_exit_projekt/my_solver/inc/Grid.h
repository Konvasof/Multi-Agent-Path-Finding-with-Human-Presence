#pragma once
#include <string>
#include <vector>
#include <set>
#include <utility> // std::pair

class GridMap {
public:
    int width = 0;
    int height = 0;
    std::set<std::pair<int, int>> walls;
    std::pair<int, int> exit_point = {-1, -1};

    GridMap() = default;
    explicit GridMap(const std::string& raw_map_content);

    void set_exit(int x, int y);
    bool in_bounds(int x, int y) const;
    bool is_walkable(int x, int y) const;

private:
    void _parse_map(const std::string& content);
};
