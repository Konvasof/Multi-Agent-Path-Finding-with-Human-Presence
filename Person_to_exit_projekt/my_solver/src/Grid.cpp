#include "Grid.h"
#include <sstream>
#include <iostream>

GridMap::GridMap(const std::string& raw_map_content) {
    _parse_map(raw_map_content);
}

void GridMap::_parse_map(const std::string& content) {
    std::stringstream ss(content);
    std::string line;
    bool parsing_grid = false;
    int y = 0;

    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        
        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (line.find("height") == 0) {
            height = std::stoi(line.substr(7));
        } else if (line.find("width") == 0) {
            width = std::stoi(line.substr(6));
        } else if (line.find("map") == 0) {
            parsing_grid = true;
            continue;
        } else if (parsing_grid) {
            for (int x = 0; x < line.length(); ++x) {
                if (line[x] == '@' || line[x] == 'T') { 
                    walls.insert({x, y});
                }
            }
            y++;
        }
    }
}

void GridMap::set_exit(int x, int y) {
    if (!in_bounds(x, y)) {
        std::cerr << "Exit " << x << "," << y << " is outside the map." << std::endl;
        return;
    }
    // Ujistíme se, že exit není ve zdi
    walls.erase({x, y});
    exit_point = {x, y};
}

bool GridMap::in_bounds(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

bool GridMap::is_walkable(int x, int y) const {
    return in_bounds(x, y) && walls.find({x, y}) == walls.end();
}
