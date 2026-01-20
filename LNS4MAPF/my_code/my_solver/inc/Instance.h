/**
 * @file
 * @brief MAPF problem instance representation and related functions.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 08-01-2025
 */

#pragma once
#include <string>
#include <vector>

#include "Map.h"
#include "utils.h"

/**
 * @brief If this is defined, the manhattan and euclidean heuristics are calculated.
 */
// #define CALCULATE_OTHER_HEURISTICS

/**
 * @brief Class representing a MAPF problem instance.
 */
class Instance
{
public:
  /**
   * @brief Constructs a blank instance of the MAPF problem.
   */
  Instance() = default;

  /**
   * @brief Constructs a MAPF problem instance from the given map and scene files.
   *
   * @param map_fname_ Name of the map file.
   * @param scene_fname_ Name of the scene file.
   * @param num_of_agents_ Number of agents, that should be loaded.
   * @param calculate_manhattan If true, the manhattan heuristic is calculated.
   * @param calculate_euclidean If true, the euclidean heuristic is calculated.
   */
  Instance(std::string map_fname_, std::string scene_fname_, int num_of_agents_ = 0, bool calculate_manhattan = false,
           bool calculate_euclidean = false);

  /**
   * @brief Initialization with map and scene files.
   *
   * @param map_fname Name of the map file.
   * @param scene_fname Name of the scene file.
   * @param num_of_agents Number of agents, that should be loaded.
   * @param calculate_manhattan If true, the manhattan heuristic is calculated.
   * @param calculate_euclidean If true, the euclidean heuristic is calculated.
   */
  void initialize(const std::string& map_fname, const std::string& scene_fname, int num_of_agents = 0, bool calculate_manhattan = false,
                  bool calculate_euclidean = false);

  /**
   * @brief Initialization function, if the map and scene files and number of agents are already specified.
   *
   * @param calculate_manhattan If true, the manhattan heuristic is calculated.
   * @param calculate_euclidean If true, the euclidean heuristic is calculated.
   */
  void initialize(bool calculate_manhattan, bool calculate_euclidean);

  /**
   * @brief Calculates the heuristics for the agents.
   *
   * @param calculate_manhattan Whether to calculate the manhattan heuristic.
   * @param calculate_euclidean Whether to calculate the euclidean heuristic.
   */
  void calculate_heuristics(bool calculate_manhattan, bool calculate_euclidean);

  /**
   * @brief Precomputes the neighbors for each location in the map.
   */
  void precompute_neighbors();

  /**
   * @brief Converts a path to a point path.
   *
   * @param path The path to be converted.
   *
   * @return  The converted point path.
   */
  [[nodiscard]] auto path_to_pointpath(const Path& path) const -> PointPath;

  /**
   * @brief Checks the validity of a timepoint path.
   *
   * @param tp_path The timepoint path to be checked.
   *
   * @return True if the timepoint path is valid, false otherwise.
   */
  [[nodiscard]] auto check_timepointpath_validity(const TimePointPath& tp_path) const -> bool;

  /**
   * @brief Resets the instance to its initial state.
   */
  void reset();

  /**
   * @brief Prints the start and goal positions of the agents.
   */
  void print_agents() const;

  /**
   * @brief Checks if the instance is initialized.
   *
   * @return  True if the instance is initialized, false otherwise.
   */
  [[nodiscard]] auto is_initialized() const -> bool
  {
    return initialized;
  }

  /**
   * @brief Prints the map data.
   */
  void print_map() const
  {
    assertm(initialized, "Instance not initialized.");
    map_data.print();
  }


  /**
   * @brief Getter for the map.
   *
   * @return  The map data.
   */
  [[nodiscard]] inline auto get_map_data() const -> const Map&
  {
    assertm(initialized, "Instance not initialized.");
    return map_data;
  }

  /**
   * @brief Getter for the start positions of the agents.
   *
   * @return
   */
  [[nodiscard]] auto get_start_positions() const -> const std::vector<Point2d>&
  {
    assertm(initialized, "Instance not initialized.");
    return start_positions;
  }

  /**
   * @brief Getter for the start locations of the agents.
   *
   * @return The start locations of the agents.
   */
  [[nodiscard]] inline auto get_start_locations() const -> const std::vector<int>&
  {
    assertm(initialized, "Instance not initialized.");
    return start_locations;
  }

  /**
   * @brief Getter for the goal positions of the agents.
   *
   * @return The goal positions of the agents.
   */
  [[nodiscard]] auto get_goal_positions() const -> const std::vector<Point2d>&
  {
    assertm(initialized, "Instance not initialized.");
    return goal_positions;
  }

  /**
   * @brief Getter for the goal locations of the agents.
   *
   * @return The goal locations of the agents.
   */
  [[nodiscard]] inline auto get_goal_locations() const -> const std::vector<int>&
  {
    assertm(initialized, "Instance not initialized.");
    return goal_locations;
  }

#ifdef CALCULATE_OTHER_HEURISTICS
  /**
   * @brief Getter for the manhattan heuristic for a given agent and location.
   *
   * @param agent_num The agent number.
   * @param loc The location.
   *
   * @return The manhattan heuristic for the given agent and location.
   * @note This function is only available if CALCULATE_OTHER_HEURISTICS is defined.
   */
  [[nodiscard]] inline auto get_heuristic_manhattan(const int agent_num, const int loc) const -> int
  {
    assertm(initialized, "Instance not initialized.");
    assertm(agent_num >= 0 && agent_num < (int)heuristic_manhattan.size(), "Agent number out of range of the heuristic vector.");
    assertm(map_data.is_in(loc), "Trying to index a point that is not in the map.");
    return heuristic_manhattan[agent_num][loc];
  };

  /**
   * @brief Getter for the euclidean heuristic for a given agent and location.
   *
   * @param agent_num The agent number.
   * @param loc The location.
   *
   * @return The euclidean heuristic for the given agent and location.
   * @note This function is only available if CALCULATE_OTHER_HEURISTICS is defined.
   */
  [[nodiscard]] auto get_heuristic_euclidean(int agent_num, int loc) const -> double
  {
    assertm(initialized, "Instance not initialized.");
    assertm(agent_num >= 0 && agent_num < (int)heuristic_euclidean.size(), "Agent number out of range of the heuristic vector.");
    assertm(map_data.is_in(loc), "Trying to index a point that is not in the map.");
    return heuristic_euclidean[agent_num][loc];
  }
#endif

  /**
   * @brief Getter for the distance heuristic for a given agent and location.
   *
   * @param agent_num The agent number.
   * @param loc The location.
   *
   * @return The distance heuristic for the given agent and location.
   */
  [[nodiscard]] inline auto get_heuristic_distance(const int agent_num, const int loc) const -> int
  {
    assertm(agent_num >= 0 && agent_num < (int)heuristic_distance.size(), "Agent number out of range of the heuristic vector.");
    assertm(map_data.is_in(loc), "Trying to index a point that is not in the map.");
    return heuristic_distance[agent_num][loc];
  };

  /**
   * @brief Finds locations of the neighbors of a given location.
   *
   * @param loc The location for which to find the neighbors.
   *
   * @return The locations of the neighbors of the given location.
   */
  [[nodiscard]] inline auto get_neighbor_locations(int loc) const -> const std::vector<int>&
  {
    assertm(map_data.is_in(loc), "Trying to get neighbors of invalid location.");
    assertm(loc < (int)neighbors.size(), "Neighbors are not precomputed for this location.");
    return neighbors[loc];
  }

  /**
   * @brief Getter for the sum of distances.
   *
   * @return The sum of distances for all agents.
   */
  [[nodiscard]] inline auto get_sum_of_distances() const -> int
  {
    assertm(initialized, "Instance not initialized.");
    return sum_of_distances;
  }

  /**
   * @brief Converts a location to a 2D point.
   *
   * @param location The location to be converted.
   *
   * @return The 2D point corresponding to the given location.
   */
  [[nodiscard]] auto location_to_position(int location) const -> Point2d
  {
    assertm(initialized, "Instance not initialized.");
    return Point2d(location % map_data.width, (int)(location / map_data.width));
  }

  /**
   * @brief Converts a 2D point to a location.
   *
   * @param position The 2D point to be converted.
   *
   * @return The location corresponding to the given 2D point.
   */
  [[nodiscard]] auto position_to_location(const Point2d& position) const -> int
  {
    assertm(initialized, "Instance not initialized.");
    return position.y * map_data.width + position.x;
  }

  /**
   * @brief Getter for the number of cells in the map.
   *
   * @return  The number of cells in the map.
   */
  [[nodiscard]] inline auto get_num_cells() const -> int
  {
    return map_data.get_num_cells();
  }

  /**
   * @brief Getter for the number of free cells in the map.
   *
   * @return The number of free cells in the map.
   */
  [[nodiscard]] inline auto get_num_free_cells() const -> int
  {
    assertm(initialized, "Instance not initialized.");
    return map_data.get_num_free_cells();
  }

  /**
   * @brief Checks if a given location is a goal location.
   *
   * @param loc The location to be checked.
   *
   * @return True if the location is a goal location, false otherwise.
   */
  [[nodiscard]] inline auto is_goal_location(int loc) const -> bool
  {
    assertm(initialized, "Instance not initialized.");
    assertm(map_data.is_in(loc), "Invalid location.");
    return location_to_goal_array[loc] != -1;
  }

  /**
   * @brief Returns the id of the agent whose goal is at the given location.
   *
   * @param loc The location to be checked.
   *
   * @return The id of the agent whose goal is at the given location. If the location is not a goal location, -1 is returned.
   */
  [[nodiscard]] inline auto whose_goal(int loc) const -> int
  {
    assertm(initialized, "Instance not initialized.");
    assertm(map_data.is_in(loc), "Invalid location.");
    assertm(loc >= 0 && loc < static_cast<int>(location_to_goal_array.size()), "Location outside the vector size");
    return location_to_goal_array[loc];
  }

  /**
   * @brief Getter for the number of agents in the instance.
   *
   * @return The number of agents in the instance.
   */
  [[nodiscard]] inline auto get_num_of_agents() const -> int
  {
    assertm(initialized, "Instance not initialized.");
    return num_of_agents;
  }

  /**
   * @brief Converts a location to a free location.
   *
   * @param loc The location to be converted.
   *
   * @return The free location corresponding to the given location.
   */
  [[nodiscard]] inline auto location_to_free_location(int loc) const -> int
  {
    assertm(initialized, "Instance not initialized.");
    return map_data.location_to_free_location(loc);
  }

  /**
   * @brief Converts a free location to a location.
   *
   * @param free_loc The free location to be converted.
   *
   * @return The location corresponding to the given free location.
   */
  [[nodiscard]] inline auto free_location_to_location(int free_loc) const -> int
  {
    assertm(initialized, "Instance not initialized.");
    return map_data.free_location_to_location(free_loc);
  }

private:
  /**
   * @brief Load the map from a file specified during Instance initialization.
   *
   * @return  True if the map was loaded successfully, false otherwise.
   */
  void load_map();

  /**
   * @brief Load the scene from a file specified during Instance initialization.
   */
  void load_scene();

  /**
   * @brief Calculate the distance heuristic for each agent and location.
   */
  void calculate_distance_heuristic();

  bool                 initialized = false;    /**< Flag indicating whether the instance is initialized. */
  std::string          map_fname;              /**< Name of the map file. */
  std::string          scene_fname;            /**< Name of the scene file. */
  Map                  map_data;               /**< The map data. */
  std::vector<Point2d> start_positions;        /**< Start positions of the agents. */
  std::vector<int>     start_locations;        /**< Start locations of the agents. */
  std::vector<Point2d> goal_positions;         /**< Goal positions of the agents. */
  std::vector<int>     goal_locations;         /**< Goal locations of the agents. */
  std::vector<int>     location_to_goal_array; /**< Array mapping locations to goal locations. */
  // std::vector<Path>             optimal_paths;
  std::vector<std::vector<int>> neighbors; /**< Neighbors of each location in the map. */

  // heuristics
#ifdef CALCULATE_OTHER_HEURISTICS
  std::vector<std::vector<int>>    heuristic_manhattan; /**< Manhatten heuristic for each agent and location. */
  std::vector<std::vector<double>> heuristic_euclidean; /**< Euclidean heuristic for each agent and location. */
#endif
  std::vector<std::vector<int>> heuristic_distance; /**< Distance heuristic for each agent and location. */

  int sum_of_distances = 0; /**< Sum of distances for all agents. */
  int num_of_agents;        /**< Number of agents in the instance. */
};

