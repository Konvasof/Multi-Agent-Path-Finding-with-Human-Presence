/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <boost/program_options.hpp>
#include <iostream>
#include <memory>
#include <fstream> 

#include "Computation.h"
#include "Instance.h"
#include "LNS.h"
#include "SIPP.h"
#include "SharedData.h"
#include "Visualizer.h"
#include "magic_enum/magic_enum.hpp"

// create program options namespace
namespace po = boost::program_options;

constexpr int    DEFAULT_MAX_ITER          = 10;
constexpr double DEFAULT_TIME_LIMIT        = 30.0;
//constexpr double DEFAULT_TIME_LIMIT        = 180.0;
constexpr int    DEFAULT_NEIGHBORHOOD_SIZE = 10;


auto main(int argc, char** argv) -> int
{
  // Define command-line options
  po::options_description desc("Allowed options");
  desc.add_options()("help,h", "Show help message")("map,m", po::value<std::string>()->required(), "file with the map")(
      "agents,a", po::value<std::string>()->required(), "input file for agents")(
      "agentNum,k", po::value<int>()->default_value(0), "number of agents")("GUI,G", po::value<bool>()->default_value(true),
                                                                            "whether to turn on the graphical user interface")(
      "sipp_suboptimality,w", po::value<double>()->default_value(1.0), "suboptimality factor for suboptimal sipp")(
      "maxIterations,i", po::value<int>()->default_value(DEFAULT_MAX_ITER), "maximal number of iterations of LNS")(
      "timeLimit,t", po::value<double>()->default_value(DEFAULT_TIME_LIMIT), "time limit to find the solution, in seconds")(
      "safetyCheck", po::value<bool>()->default_value(false), "Enable safety-aware LNS mode")(
      "humanPath", po::value<std::string>()->default_value(""), "Path to human path file")(
      "safetyDoor", po::value<int>()->default_value(-1), "Location ID of the safety door")(
      "sipp_implementation", po::value<std::string>()->default_value("SIPP_mine"),
      "implementation of SIPP (SIPP_mine, SIPP_mapf_lns, SIPP_suboptimal)")("Restarts,r", po::value<bool>()->default_value(true),
                                                                            "restart the search if no feasible initial solution was found")(
      "destroy_operator", po::value<std::string>()->default_value("ADAPTIVE"),
      "Destroy operator to be used in LNS (RANDOM, RANDOMWALK, INTERSECTION, ADAPTIVE, RANDOM_CHOOSE, BLOCKED)")(
      "neighborhood_size,n", po::value<int>()->default_value(DEFAULT_NEIGHBORHOOD_SIZE),
      "Size of the neighborhood used by the destroy operator (number of paths to be destroyed)")(
      "humanStartX", po::value<int>()->default_value(-1), "Human Start X coordinate")(
      "humanStartY", po::value<int>()->default_value(-1), "Human Start Y coordinate")(
      "seed,s", po::value<int>()->default_value(-1),
      "seed of the random generators for reproducability, to achieve non reproducible random behavior, use negative value")(
      "output_paths", po::value<std::string>()->default_value(""),
      "Output file for the paths of the generated solution. If not used, the solution will not be exported.");

  // Parse command-line arguments
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

  // create the instance
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(map_name, scene_name, agent_num);

  // calculate the optimal paths neglecting the other agents
  // instance->calculate_optimal_paths_parallel(); // old, dont use
  // instance->calculate_optimal_paths();

  // create the shared datastructure
  std::unique_ptr<SharedData> shared_data = std::make_unique<SharedData>();

  // create the info type
  INFO_type info_type = INFO_type::no_info;
  if (GUI)
  {
    info_type = INFO_type::visualisation;
  }

  // read sipp implementation
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

  // read destroy operator
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

  if (neighborhood_size < 1 || neighborhood_size > agent_num)
  {
    throw std::runtime_error("Invalid neighborhood size");
  }


  // create SIPP settings
  SIPP_settings sipp_settings = SIPP_settings(sipp_implementation, info_type, w);

  // create destroy settings
  Destroy_settings destroy_settings = Destroy_settings(destroy_type, neighborhood_size);

  // create LNS settings
  LNS_settings lns_settings = LNS_settings(max_iter, time_limit, destroy_settings, sipp_settings, restarts);


  // create the computation object
  Computation computation(*instance, shared_data.get(), lns_settings, seed);

  bool safety_aware = vm["safetyCheck"].as<bool>();
  std::string human_file = vm["humanPath"].as<std::string>();
  int safety_door = vm["safetyDoor"].as<int>();

  // If a user provides a starting position, it is loaded from the terminal
  int h_start_x = vm["humanStartX"].as<int>();
  int h_start_y = vm["humanStartY"].as<int>();
  int human_start_loc = -1;

  // If a starting position is not provided, it is generated randomly
  if (h_start_x != -1 && h_start_y != -1) {
      // Ensuring that the position is inside the map
      if (instance->get_map_data().is_in({h_start_x, h_start_y})) {
           human_start_loc = instance->position_to_location({h_start_x, h_start_y});
           
           // Ensuring the user does not send the person into a blocked area
           if (instance->get_map_data().data[human_start_loc] != 0) {
               std::cout << "WARNING: User defined human start is on an obstacle/door! Resetting to random." << std::endl;
               human_start_loc = -1;
           }
           // Ensuring the user does not send person on the same position as a starting point of the robot
           if (human_start_loc != -1) { 
               const auto& agent_starts = instance->get_start_locations();
               for (int robot_loc : agent_starts) {
                   if (robot_loc == human_start_loc) {
                       std::cout << "WARNING: User defined human start is on a robot! Resetting to random choice." << std::endl;
                       human_start_loc = -1;
                       break;
                   }
               }
           }
      }
      else {
          std::cout << "WARNING: User defined coordinates are out of map bounds! Resetting to random choice." << std::endl;
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
      
      int max_attempts = 10000;
      bool position_found = false;

      const auto& agent_starts = instance->get_start_locations();
      std::unordered_set<int> agent_start_set(agent_starts.begin(), agent_starts.end());

      // Iterating throught a map
      for (int i = 0; i < max_attempts; i++) {
          int candidate_loc = dist(gen);
          // Not same as a wall or door
          if (map_data.data[candidate_loc] == 0) {
              
              // Not same as a start of robot
              if (agent_start_set.find(candidate_loc) == agent_start_set.end()) {
                  human_start_loc = candidate_loc;
                  position_found = true;
                  
                  auto pos = instance->location_to_position(human_start_loc);
                  std::cout << "Human spawn set to: [" << pos.x << ", " << pos.y << "] (Location ID: " << human_start_loc << ")" << std::endl;
                  break;
              }
          }
      }

      if (!position_found) {
          std::cerr << "ERROR: Failed to generate valid human position after " << max_attempts << " attempts." << std::endl;
      }
  }
  if (safety_door == -1) {
      std::cout << "Searching for safety door ('2') in the map..." << std::endl;
      const auto& map_data = instance->get_map_data();
      std::vector<int> door_locations;
      
      // Try to find all doors directly from the map
      for (int i = 0; i < (int)map_data.data.size(); i++) {
          if (map_data.data[i] == 2) {
              door_locations.push_back(i);
          }
      }
      
      if (!door_locations.empty()) {
          // If there are doors in the map, choose one randomly
          std::mt19937 gen_door;
          if (seed == -1) {
              std::random_device rd;
              gen_door.seed(rd());
          } else {
              gen_door.seed(seed + 99); // Different seed to avoid affecting other random generators
          }
          std::uniform_int_distribution<> dist_door(0, door_locations.size() - 1);
          safety_door = door_locations[dist_door(gen_door)];
          
          auto pos = instance->location_to_position(safety_door);
          std::cout << "Safety door auto-detected at: [" << pos.x << ", " << pos.y << "] (Location ID: " << safety_door << ")" << std::endl;
      } else {
          // 2. Fallback: No '2' found in the map, generate a random door on the edge
          std::cout << "No doors found. Generating a random door position on the edge..." << std::endl;
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

  return EXIT_SUCCESS;
}
