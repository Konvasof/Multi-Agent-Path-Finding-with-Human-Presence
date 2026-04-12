#include <boost/program_options.hpp> 
#include <iostream>
#include <memory>
#include <fstream> 
#include <iomanip>
#include <nlohmann/json.hpp>

#include "Computation.h"
#include "Instance.h"
#include "LNS.h"
#include "SIPP.h"
#include "SharedData.h"
#include "Visualizer.h"
#include "magic_enum/magic_enum.hpp"

// Create program options namespace
namespace po = boost::program_options;

using json = nlohmann::json;

constexpr int    DEFAULT_MAX_ITER          = 100;
constexpr double DEFAULT_TIME_LIMIT        = 30.0;
constexpr int    DEFAULT_NEIGHBORHOOD_SIZE = 10;

auto main(int argc, char** argv) -> int
{
  // Define command-line options
  po::options_description desc("Allowed options");
  desc.add_options()(
      "help,h", "Show help message")(
      "map,m", po::value<std::string>()->required(), "file with the map")(
      "agents,a", po::value<std::string>()->required(), "input file for agents")(
      "agentNum,k", po::value<int>()->default_value(0), "number of agents")(
      "GUI,G", po::value<bool>()->default_value(true),"whether to turn on the graphical user interface")(
      "sipp_suboptimality,w", po::value<double>()->default_value(1.0), "suboptimality factor for suboptimal sipp")(
      "maxIterations,i", po::value<int>()->default_value(DEFAULT_MAX_ITER), "maximal number of iterations of LNS")(
      "timeLimit,t", po::value<double>()->default_value(DEFAULT_TIME_LIMIT), "time limit to find the solution, in seconds")(
      "safetyCheck", po::value<bool>()->default_value(false), "Enable safety-aware LNS mode")(
      "humanPath", po::value<std::string>()->default_value(""), "Path to human path file")(
      "safetyDoor", po::value<int>()->default_value(-1), "Location ID of the safety door")(
      "sipp_implementation", po::value<std::string>()->default_value("SIPP_mine"),
      "implementation of SIPP (SIPP_mine, SIPP_mapf_lns, SIPP_suboptimal)")(//set as default SIPP_mine, but can be changed to SIPP_suboptimal for suboptimal SIPP
      "Restarts,r", po::value<bool>()->default_value(true),"restart the search if no feasible initial solution was found")(
      "destroy_operator", po::value<std::string>()->default_value("RANDOM"),
      "Destroy operator to be used in LNS (RANDOM, RANDOMWALK, INTERSECTION, ADAPTIVE, RANDOM_CHOOSE, BLOCKED)")(
      "neighborhood_size,n", po::value<int>()->default_value(DEFAULT_NEIGHBORHOOD_SIZE),
      "Size of the neighborhood used by the destroy operator (number of paths to be destroyed)")(
      "humanStartX", po::value<int>()->default_value(-1), "Human Start X coordinate")(
      "humanStartY", po::value<int>()->default_value(-1), "Human Start Y coordinate")(
      "seed,s", po::value<int>()->default_value(-1),
      "seed of the random generators for reproducability, to achieve non reproducible random behavior, use negative value")(
      "output_paths", po::value<std::string>()->default_value(""),
      "Output file for the paths of the generated solution. If not used, the solution will not be exported.");

  // Parse and validate command-line arguments
  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  }
  catch (const po::error& ex)
  {
    // Handle --help
    if (vm.count("help") != 0U)
    {
      std::cout << desc << std::endl;
      return 0;
    }
    // Handle missing arguments
    std::cerr << "Error: " << ex.what() << "\n";
    return 1;
  }

  const std::string map_name          = vm["map"].as<std::string>();
  const std::string scene_name        = vm["agents"].as<std::string>();
  const std::string sipp_algo         = vm["sipp_implementation"].as<std::string>();
  const std::string destroy_name      = vm["destroy_operator"].as<std::string>();
  const std::string output_paths_file = vm["output_paths"].as<std::string>();
  const int         neighborhood_size = vm["neighborhood_size"].as<int>();
  const int         agent_num         = vm["agentNum"].as<int>();
  const int         max_iter          = vm["maxIterations"].as<int>();
  const double      time_limit        = vm["timeLimit"].as<double>();
  const bool        GUI               = vm["GUI"].as<bool>();
  const bool        restarts          = vm["Restarts"].as<bool>();
  const double      w                 = vm["sipp_suboptimality"].as<double>();
  const int         seed              = vm["seed"].as<int>();

  // Create the instance based on map and scene files
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(map_name, scene_name, agent_num);

  // Create the shared datastructure
  std::unique_ptr<SharedData> shared_data = std::make_unique<SharedData>();

  // Create the info type
  INFO_type info_type = INFO_type::no_info;
  if (GUI)
  {
    info_type = INFO_type::visualisation;
  }

  // Read sipp implementation ,default is SIPP_mine
  SIPP_implementation sipp_implementation     = SIPP_implementation::SIPP_mine;
  auto                sipp_implementation_opt = magic_enum::enum_cast<SIPP_implementation>(sipp_algo, magic_enum::case_insensitive);
  if (sipp_implementation_opt.has_value())
  {
    sipp_implementation = sipp_implementation_opt.value();
  }
  else
  {
    std::cout << "WARNING: Unknown sipp implementation: '" << sipp_algo << "', using default option "
              << magic_enum::enum_name(sipp_implementation) << std::endl;
  }
  // Read destroy operator, default is RANDOM
  DESTROY_TYPE destroy_type     = DESTROY_TYPE::RANDOM;
  auto         destroy_type_opt = magic_enum::enum_cast<DESTROY_TYPE>(destroy_name, magic_enum::case_insensitive);
  if (destroy_type_opt.has_value())
  {
    destroy_type = destroy_type_opt.value();
  }
  else
  {
    std::cout << "WARNING: Unknown destroy type: '" << destroy_name << "', using default option " << magic_enum::enum_name(destroy_type)
              << std::endl;
  }

  // Sanity check for neighborhood size
  if (neighborhood_size < 1 || neighborhood_size > agent_num)
  {
    throw std::runtime_error("Invalid neighborhood size");
  }

  // Create SIPP settings agregation
  SIPP_settings sipp_settings = SIPP_settings(sipp_implementation, info_type, w);

  // Create destroy settings agregation
  Destroy_settings destroy_settings = Destroy_settings(destroy_type, neighborhood_size);

  // Create LNS settings agregation
  LNS_settings lns_settings = LNS_settings(max_iter, time_limit, destroy_settings, sipp_settings, restarts);

  // Create the computation object
  Computation computation(*instance, shared_data.get(), lns_settings, seed);

  // Extraction of safety parameters
  bool safety_aware = vm["safetyCheck"].as<bool>(); //If true, the safety-aware LNS mode is enabled
  std::string human_file = vm["humanPath"].as<std::string>(); //If a user provides a human path file
  int safety_door = vm["safetyDoor"].as<int>(); // If a user provides a safety door location

  // If a user provides a starting position, it is loaded from the terminal
  int h_start_x = vm["humanStartX"].as<int>();
  int h_start_y = vm["humanStartY"].as<int>();
  int human_start_loc = -1;
  // Try to find a human starting position in the map
  int map_human_loc = instance->get_parsed_human_location();

  if (map_human_loc != -1) {
      human_start_loc = map_human_loc;
      auto pos = instance->location_to_position(human_start_loc);
      std::cout << "Human spawn loaded directly from the .map file at: [" << pos.x << ", " << pos.y << "]" << std::endl;
  }
  // If a starting position is not provided, it is generated randomly or directly from the terminal input
  else if (h_start_x != -1 && h_start_y != -1) {
    // Ensuring that the position is inside the map
    if (instance->get_map_data().is_in({h_start_x, h_start_y})) {
        int candidate_loc = instance->position_to_location({h_start_x, h_start_y});
        // Ensuring the user does not send the person into a blocked area
        if (instance->get_map_data().data[candidate_loc] != 0) {
            std::cout << "WARNING: Terminal human start is on an obstacle! Falling back to random choice." << std::endl;
        }
        else {
            bool collision_with_robot = false;
            for (int robot_loc : instance->get_start_locations()) {
                if (robot_loc == candidate_loc) {
                    collision_with_robot = true;
                    break;
                }
            }
            if (collision_with_robot) {
                  std::cout << "WARNING: Terminal human start is on a robot! Falling back to random choice." << std::endl;
            } 
            else {
                  human_start_loc = candidate_loc;
                  std::cout << "Human spawn loaded from terminal arguments at: [" << h_start_x << ", " << h_start_y << "]" << std::endl;
            }
        }
    }
    else {
          std::cout << "WARNING: Terminal coordinates are out of map bounds! Falling back to random choice." << std::endl;
    }
  }       
  // If it's neccesary to make a random position
  if (human_start_loc == -1) {
      std::cout << "Generating a random human position..." << std::endl;
      
      // Keep a seed 
      std::mt19937 gen;
      if (seed == -1) {
          std::random_device rd;
          gen.seed(rd());
      } else {
          gen.seed(seed + 42); // Not same seed as for agents
      }

      const auto& map_data = instance->get_map_data();
      std::uniform_int_distribution<> dist(0, map_data.data.size() - 1);
      
      const auto& agent_starts = instance->get_start_locations();
      std::unordered_set<int> agent_start_set(agent_starts.begin(), agent_starts.end());

      int max_attempts = 10000;
      bool position_found = false;

      // Iterating throught a map
      for (int i = 0; i < max_attempts; i++) {
          int candidate_loc = dist(gen);
          // Not same as a wall or door
          if (map_data.data[candidate_loc] == 0 && agent_start_set.find(candidate_loc) == agent_start_set.end()) {
            human_start_loc = candidate_loc;
            position_found = true;                 
            auto pos = instance->location_to_position(human_start_loc);
            std::cout << "Human spawn randomly set to: [" << pos.x << ", " << pos.y << "] (Location ID: " << human_start_loc << ")" << std::endl;
            break;
              
          }
      }
      if (!position_found) {
          std::cerr << "ERROR: Failed to generate valid human position after " << max_attempts << " attempts." << std::endl;
      }
    }

    int map_door_loc = instance->get_parsed_safety_door();
    
    if (map_door_loc != -1) {
        safety_door = map_door_loc;
        auto pos = instance->location_to_position(safety_door);
        std::cout << "Safety door loaded directly from the .map file at: [" << pos.x << ", " << pos.y << "]" << std::endl;
    } else if (safety_door == -1) {
        // 2. Fallback: No '2' found in the map, generate a random door on the edge
        std::cout << "No doors found. Generating a random door position on the edge..." << std::endl;
        const auto& map_data = instance->get_map_data();
        std::mt19937 gen_door;
        if (seed == -1) {
            std::random_device rd;
            gen_door.seed(rd());
        } else {
            gen_door.seed(seed + 99);
        }
    
        std::unordered_set<int> robot_occupied_locs;
        for (int loc : instance->get_start_locations()) robot_occupied_locs.insert(loc);
        for (int loc : instance->get_goal_locations()) robot_occupied_locs.insert(loc);

        int width = map_data.width;
        int height = map_data.height;
        std::vector<int> valid_edge_doors;

        // Find all cells that are on the edge, not in the corners, and are free (0)
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                bool is_edge = (x == 0 || x == width - 1 || y == 0 || y == height - 1);
                bool is_corner = ((x == 0 && y == 0) || (x == 0 && y == height - 1) ||
                                (x == width - 1 && y == 0) || (x == width - 1 && y == height - 1));
                
                if (is_edge && !is_corner) {
                    int loc = y * width + x;
                    // Must be free space (0) and not the human's start location
                    if (map_data.data[loc] == 0 && loc != human_start_loc && robot_occupied_locs.find(loc) == robot_occupied_locs.end()) {
                        valid_edge_doors.push_back(loc);
                    }
                }
            }
        }

        if (!valid_edge_doors.empty()) {
            // Pick a random door from the valid edge locations
            std::uniform_int_distribution<> dist_door(0, valid_edge_doors.size() - 1);
            safety_door = valid_edge_doors[dist_door(gen_door)];
            
            auto pos = instance->location_to_position(safety_door);
            std::cout << "Random edge safety door generated at: [" << pos.x << ", " << pos.y << "] (Location ID: " << safety_door << ")" << std::endl;
        } else {
            // Ultimate fallback in case the map is completely closed off by walls on all edges
            std::cerr << "WARNING: No free edge locations found! Falling back to any free space..." << std::endl;
            std::uniform_int_distribution<> dist_map(0, map_data.data.size() - 1);
            int max_attempts = 10000;
            bool door_found = false;
            for (int i = 0; i < max_attempts; i++) {
                int candidate_loc = dist_map(gen_door);
                if (map_data.data[candidate_loc] == 0 && candidate_loc != human_start_loc) {
                    safety_door = candidate_loc;
                    door_found = true;
                    auto pos = instance->location_to_position(safety_door);
                    std::cout << "Random fallback safety door generated at: [" << pos.x << ", " << pos.y << "] (Location ID: " << safety_door << ")" << std::endl;
                    break;
                }
            }
            if (!door_found) {
                std::cerr << "ERROR: Failed to generate valid door position!" << std::endl;
            }
        }
    }
   
    std::vector<int> human_path_locs;
    if (!human_file.empty())
    {
        std::ifstream hf(human_file);
        if (hf.is_open())
        {
            int x, y;
            while (hf >> x >> y)
            {
                if (instance->get_map_data().is_in({x, y}))
                {
                    // Převedeme souřadnice (x,y) na Location ID
                    human_path_locs.push_back(instance->position_to_location({x, y}));
                }
            }
        }
        else
        {
            std::cout << "WARNING: Could not open human path file: " << human_file << std::endl;
        }
    }
    shared_data->safety_door_location = safety_door;
    computation.set_safety_params(safety_aware, human_start_loc, safety_door);

    // start the computation thread
    computation.start();

    // create the visualization object
    if (GUI)
    {
        Visualizer visualizer(*instance, computation, *shared_data, seed + 1);  // use different seed for visualizer than for computation
        //visualizer.start();
        //visualizer.load_human_path("human_path.txt");
        
        visualizer.run();
        // wait for the visualization thread to finish
        //visualizer.join_thread();
    }

    // wait for the computation thread to finish
    computation.join_thread();

    // export the solution
    if (!output_paths_file.empty())
    {
        const Solution& sol = computation.get_solution();
        sol.save(output_paths_file, *instance);
    }
  
  // Logging the results to a JSON file
  const Solution& sol = computation.get_solution();
  json result;

  // Gain metadata
  result["experiment"]["map"] = map_name;
  result["experiment"]["scenario"] = scene_name;
  result["experiment"]["agents"] = agent_num;
  result["experiment"]["time_limit"] = time_limit;
  result["experiment"]["max_iterations"] = max_iter;
  result["experiment"]["seed"] = seed;
  result["experiment"]["sipp_algo"] = sipp_algo;
  result["experiment"]["destroy_operator"] = destroy_name;

  // Safety parameters
  result["safety"]["enabled"] = safety_aware;
  result["safety"]["human_start_loc"] = human_start_loc;
  result["safety"]["safety_door_loc"] = safety_door;

  // Results
  result["results"]["feasible"] = sol.feasible;
  
  if (sol.feasible) {
      result["results"]["sum_of_costs"] = sol.sum_of_costs;
      result["results"]["makespan"] = sol.makespan;
      result["results"]["sum_of_delays"] = sol.sum_of_delays;
  } else {
      result["results"]["sum_of_costs"] = -1;
  }

  // Name of the map
  std::filesystem::path p(map_name);
  std::string clean_map_name = p.stem().string(); 
  
  std::string log_filename = "log_" + clean_map_name + "_" + std::to_string(agent_num) + "agents.json";

  // Writing the JSON
  std::ofstream out_file(log_filename);
  if (out_file.is_open()) {
      out_file << std::setw(4) << result << std::endl;
      out_file.close();
      std::cout << "\nZáznam o experimentu úspěšně uložen do: " << log_filename << " <<<" << std::endl;
  } else {
      std::cerr << "ERROR: Nepodařilo se vytvořit logovací soubor: " << log_filename << std::endl;
  }
  // zavolam ze sol cenu a feasible a meta data( jmeno instance, casovy limit, jmeno mapy atd..)
  // volam s novou cestou souboru kam se mi ulozi kazdy tenhle kolobeh
  // na konci mainu soubor, kam tohle vsechno ulozim - json soubor z kazdeho exporimnetu 
  // yaml nebo csv
  // knihovna nlohmann/json 
  // zadefinuju nlohmannjson results objekt a pak tam vkladam data result["cost"]= 1234
  // result.save(path)
  // kouknout se na to, co se deje když to není feasible, kdyz najdu reseni, neni dobry a jdu na dalsi reseni, koukni na to.
  // over ze mi dobre funguje to reseni LNS a ze to dobre prohledava 
    return EXIT_SUCCESS;
}
