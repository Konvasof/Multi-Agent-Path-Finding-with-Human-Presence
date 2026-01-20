#include "LNS.h"
#include "Replanner.h"
#include "PathFinder.h" // Pro nalezení cesty člověka
#include <iostream>
#include <algorithm>

LNS::LNS(GridMap& g, std::pair<int, int> h_start, std::pair<int, int> e_pos)
    : grid(g), human_start(h_start), exit_pos(e_pos) 
{
    // 1. Najdeme ideální cestu pro člověka (bez ohledu na roboty)
    // To je ta naše "zakázaná zóna" pro roboty
    AStarPathFinder finder;
    auto res = finder.find_path(human_start, exit_pos, grid, {});
    if (res) {
        human_ideal_path = *res;
    }
}

void LNS::load_paths(const std::map<int, std::vector<std::pair<int, int>>>& paths) {
    robot_paths = paths;
}

std::map<int, std::vector<std::pair<int, int>>> LNS::get_paths() const {
    return robot_paths;
}

std::vector<int> LNS::find_blockers(int t) {
    std::vector<int> blockers;
    if (t >= human_ideal_path.size()) return blockers;

    // Kde je člověk v čase t?
    std::pair<int, int> h_pos = human_ideal_path[t];

    // Projdu všechny roboty a zjistím, kdo tam stojí
    for (const auto& [id, path] : robot_paths) {
        std::pair<int, int> r_pos;
        if (t < path.size()) r_pos = path[t];
        else r_pos = path.back();

        if (r_pos == h_pos) {
            blockers.push_back(id);
        }
    }
    return blockers;
}

void LNS::solve(int max_iterations) {
    std::cout << "--- START LNS ---" << std::endl;
    
    // Zjistíme maximální čas simulace
    int max_time = 0;
    for(auto& p : robot_paths) max_time = std::max(max_time, (int)p.second.size());

    // Projdeme čas od 0 do konce
    for (int t = 0; t < max_time + 10; ++t) {
        
        // Zjistíme, jestli v tomto čase někdo neblokuje člověka
        std::vector<int> blockers = find_blockers(t);

        if (!blockers.empty()) {
            std::cout << "KONFLIKT v case " << t << "! Blokuji agenti: ";
            for(int id : blockers) std::cout << id << " ";
            std::cout << std::endl;

            // Opravíme každého hříšníka
            for (int agent_id : blockers) {
                std::cout << " -> Preplanuji agenta " << agent_id << "..." << std::endl;

                // Odstraníme jeho starou cestu z mapy, aby sám sobě nepřekážel
                // (Replanner se dívá do robot_paths, tak ji dočasně smažeme/ignorujeme)
                // Ale Replanner v mé implementaci má "if (other_id == agent_id) continue",
                // takže to ani nemusíme mazat.

                std::pair<int, int> start = robot_paths[agent_id][0];
                std::pair<int, int> goal = robot_paths[agent_id].back();

                // Hledáme novou cestu
                auto new_path = Replanner::find_path(
                    agent_id, start, goal, grid, robot_paths, human_ideal_path, max_time + 20
                );

                if (new_path) {
                    robot_paths[agent_id] = *new_path;
                    std::cout << " -> OK! Nova cesta nalezena." << std::endl;
                } else {
                    std::cout << " -> CHYBA! Agenta " << agent_id << " se nepodarilo odklonit." << std::endl;
                }
            }
        }
    }
}
