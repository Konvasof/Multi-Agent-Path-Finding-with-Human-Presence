#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <sstream>
#include <map>
#include <set>
#include <thread> 

// Hlavičky
#include "Grid.h"
#include "DataLoader.h"
#include "PathFinder.h"
#include "MapModifier.h"
#include "LNS.h"
#include "Replanner.h"

// Vizualizéru
#include "Visualizer.h"
#include "Instance.h"
#include "SharedData.h"
#include "Computation.h" 
#include "IterInfo.h" 
#include "Solver.h" 

// Cesty
const std::string MAP_FOLDER = "../../maps_exit_person";
const std::string LOG_FOLDER = "../../output_paths";
const std::string MAP_FILENAME = "maze-32-32-2_exit_person.map";
const std::string LOG_FILENAME = "test";

std::string load_file_content(std::string path) {
    std::ifstream t(path);
    if (!t.is_open()) throw std::runtime_error("Cannot open file: " + path);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

// Funkce pro převod cest
std::vector<std::vector<int>> convert_paths_for_vis(
    const std::map<int, std::vector<std::pair<int, int>>>& paths_map, 
    int width) 
{
    if (paths_map.empty()) return {};
    int max_id = paths_map.rbegin()->first;
    std::vector<std::vector<int>> converted(max_id + 1);

    for (const auto& [aid, path] : paths_map) {
        for (const auto& p : path) {
            converted[aid].push_back(p.second * width + p.first);
        }
    }
    return converted;
}

int main() {
    try {
        // NAČTENÍ DAT
        std::string map_path = MAP_FOLDER + "/" + MAP_FILENAME;
        std::string raw_map = load_file_content(map_path);
        GridMap grid(raw_map);
        
        std::pair<int, int> human_pos = {-1, -1}, exit_pos = {-1, -1};
        std::stringstream ss(raw_map);
        std::string line;
        bool parsing = false; int y = 0;
        while(std::getline(ss, line)) {
            if (line.find("map") == 0) { parsing = true; continue; }
            if (parsing) {
                int px = line.find('!'); if (px != std::string::npos) human_pos = {px, y};
                int ex = line.find('X'); if (ex != std::string::npos) exit_pos = {ex, y};
                y++;
            }
        }
        grid.set_exit(exit_pos.first, exit_pos.second);
        std::cout << "Mapa nactena. Human: " << human_pos.first << "," << human_pos.second << std::endl;

        std::string log_path = LOG_FOLDER + "/" + LOG_FILENAME;
        std::map<int, std::vector<std::pair<int, int>>> initial_paths;
        std::stringstream log_ss(load_file_content(log_path));
        std::string log_line;
        while(std::getline(log_ss, log_line)) {
            if(log_line.find("Agent") != std::string::npos) {
                int id_end = log_line.find(":");
                int agent_id = std::stoi(log_line.substr(6, id_end - 6));
                std::vector<std::pair<int, int>> p;
                size_t pos = id_end;
                while((pos = log_line.find("(", pos)) != std::string::npos) {
                    size_t end = log_line.find(")", pos);
                    size_t comma = log_line.find(",", pos);
                    int cx = std::stoi(log_line.substr(pos+1, comma-(pos+1)));
                    int cy = std::stoi(log_line.substr(comma+1, end-(comma+1)));
                    p.push_back({cx, cy});
                    pos = end;
                }
                initial_paths[agent_id] = p;
            }
        }

        // VÝPOČET LNS 
        LNS lns(grid, human_pos, exit_pos);
        lns.load_paths(initial_paths);
        lns.solve(1); 
        auto fixed_paths = lns.get_paths(); 
        std::cout << "LNS dokonceno." << std::endl;

        // PŘÍPRAVA GUI 
        std::cout << "Spoustim Vizualizaci..." << std::endl;

        Instance vis_instance(map_path, "", 0);
        SharedData shared_data;
        
        Computation dummy_computation(vis_instance, shared_data); 

        Visualizer visualizer(vis_instance, dummy_computation, shared_data, 42);

        // PŘEDÁNÍ DAT DO VIZUALIZÉRU
        
        //Vytvoření Solution objektu 
        Solution sol; 
        sol.sum_of_costs = 0; // Jen placeholder
        sol.converted_paths = convert_paths_for_vis(fixed_paths, grid.width);
        
        // Vytvoření SIPPInfo 
        std::vector<SIPPInfo> dummy_sipp;

        // Argumenty: iterace, accepted, improvement, sipp_info, solution, nazev_strategie
        LNSIterationInfo info(1, true, 0, dummy_sipp, sol, "MyLNS");

        // Odeslání do fronty
        shared_data.lns_info_queue.push_back(info); 
        shared_data.is_new_info.store(true);

        // START 
        visualizer.run();

    } catch (const std::exception& e) {
        std::cerr << "Chyba: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
