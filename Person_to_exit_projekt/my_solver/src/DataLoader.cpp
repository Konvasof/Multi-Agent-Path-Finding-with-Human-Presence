#include "DataLoader.h"
#include <sstream>
#include <regex>
#include <iostream>
#include <algorithm>

LogData::LogData(const std::string& raw_log_content) {
    _parse_data(raw_log_content);
}

void LogData::_parse_data(const std::string& raw_data) {
    std::stringstream ss(raw_data);
    std::string line;
    std::vector<std::vector<std::pair<int, int>>> all_agent_paths;

    std::regex coords_regex(R"(\((\d+),(\d+)\))");
    // ID agenta ze začátku řádku "Agent 0:"
    std::regex agent_id_regex(R"(Agent (\d+):)");

    while (std::getline(ss, line)) {
        if (line.empty()) continue;

        // Získat ID agenta
        std::smatch id_match;
        int current_agent_id = -1;
        if (std::regex_search(line, id_match, agent_id_regex)) {
            current_agent_id = std::stoi(id_match[1]);
        }

        // Získat cestu
        std::vector<std::pair<int, int>> path;
        auto words_begin = std::sregex_iterator(line.begin(), line.end(), coords_regex);
        auto words_end = std::sregex_iterator();

        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::smatch match = *i;
            path.push_back({std::stoi(match[1]), std::stoi(match[2])});
        }

        if (!path.empty()) {
            // Uložení do lookup tabulky pro identifikaci agentů
            if (current_agent_id != -1) {
                for (size_t t = 0; t < path.size(); ++t) {
                    agent_lookup[{static_cast<int>(t), path[t].first, path[t].second}] = current_agent_id;
                }
            }
            all_agent_paths.push_back(path);
        }
    }

    if (all_agent_paths.empty()) return;

    // Zjistit max čas
    int max_t_logged = 0;
    for (const auto& p : all_agent_paths) {
        if (!p.empty()) max_t_logged = std::max(max_t_logged, (int)p.size() - 1);
    }

    // Naplnit obstacles_at_time (včetně "zůstávání na místě" po konci cesty)
    int agent_idx = 0; // Fallback pokud regex ID selže
    for (const auto& path : all_agent_paths) {
        
        int t_last = path.size() - 1;
        std::pair<int, int> pos_last = path.back();

        // Pohyb po trase
        for (int t = 0; t < path.size(); ++t) {
            obstacles_at_time[t].push_back(path[t]);
        }

        // Stání na konci do max_time
        for (int t = t_last + 1; t <= max_t_logged; ++t) {
            obstacles_at_time[t].push_back(pos_last);
            // Musíme aktualizovat i lookup, protože agent tam stále stojí
            int id = get_agent_at(t_last, pos_last); 
            if (id != -1) agent_lookup[{t, pos_last.first, pos_last.second}] = id;
        }
        agent_idx++;
    }
}

std::set<std::pair<int, int>> LogData::get_obstacles(int time_t) {
    if (obstacles_at_time.find(time_t) == obstacles_at_time.end()) {
        return {};
    }
    std::vector<std::pair<int, int>>& vec = obstacles_at_time[time_t];
    return std::set<std::pair<int, int>>(vec.begin(), vec.end());
}

int LogData::get_max_time() const {
    if (obstacles_at_time.empty()) return 0;
    return obstacles_at_time.rbegin()->first;
}

int LogData::get_agent_at(int time, std::pair<int, int> pos) {
    auto key = std::make_tuple(time, pos.first, pos.second);
    if (agent_lookup.find(key) != agent_lookup.end()) {
        return agent_lookup[key];
    }
    return -1; // Nenalezen
}
