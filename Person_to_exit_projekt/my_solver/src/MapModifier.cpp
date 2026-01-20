#include "MapModifier.h"
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iostream>

MapModifier::MapModifier(const std::string& content, std::string fname) 
    : raw_content(content), input_filename(fname), grid(content) {
    std::random_device rd;
    rng = std::mt19937(rd());
}

std::pair<int, int> MapModifier::find_random_walkable() {
    std::vector<std::pair<int, int>> spots;
    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            if (grid.is_walkable(x, y)) spots.push_back({x, y});
        }
    }
    if (spots.empty()) throw std::runtime_error("No walkable spots!");
    
    std::uniform_int_distribution<> dist(0, spots.size() - 1);
    return spots[dist(rng)];
}

std::pair<int, int> MapModifier::find_random_edge() {
    std::vector<std::pair<int, int>> spots;
    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            bool is_edge = (x == 0 || x == grid.width - 1 || y == 0 || y == grid.height - 1);
            if (is_edge && grid.is_walkable(x, y)) spots.push_back({x, y});
        }
    }
    if (spots.empty()) throw std::runtime_error("No edge spots!");
    
    std::uniform_int_distribution<> dist(0, spots.size() - 1);
    return spots[dist(rng)];
}

std::tuple<std::pair<int, int>, std::pair<int, int>, std::string, std::string> MapModifier::generate() {
    auto human_pos = find_random_walkable();
    auto exit_pos = find_random_edge();

    // Zajistit, že nejsou stejné
    while (human_pos == exit_pos) {
        human_pos = find_random_walkable();
    }

    // Úprava textové mapy
    std::stringstream ss(raw_content);
    std::string line;
    std::stringstream output_ss;
    bool parsing_grid = false;
    int y = 0;

    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back(); // Fix CRLF
        
        if (line.find("map") == 0) {
            parsing_grid = true;
            output_ss << line << "\n";
            continue;
        }

        if (parsing_grid && y < grid.height) {
            std::string new_line = line;
            if (y == human_pos.second && human_pos.first < new_line.size()) 
                new_line[human_pos.first] = '!';
            if (y == exit_pos.second && exit_pos.first < new_line.size()) 
                new_line[exit_pos.first] = 'X';
            
            output_ss << new_line << "\n";
            y++;
        } else {
            output_ss << line << "\n";
        }
    }

    std::string base_name = input_filename.substr(0, input_filename.find_last_of('.'));
    std::string new_filename = base_name + "_exit_person.map";

    return {human_pos, exit_pos, new_filename, output_ss.str()};
}
