/**
 * @file
 * @brief Contains classes relevant to the SafeIntervalTable, which stores the safe intervals for SIPP.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 14-02-2025
 */

#pragma once
#include <climits>
#include <iostream>
#include <list>
#include <memory>
#include <unordered_set>
#include <vector>

#include "Instance.h"
#include "magic_enum/magic_enum.hpp"
#include "utils.h"

/**
 * @brief A constant for the number of directions in the grid.
 *
 * @return The number of directions in the grid.
 * @note The NONE direction is not considered in this count.
 */
constexpr int NUM_DIRECTIONS = magic_enum::enum_count<Direction>() - 1;

/**
 * @brief If this is defined, the solver is compiled with parallelized versions of the SafeIntervalTable.
 */
#define SIT_PARALLELIZATION

/**
 * @brief A class representing an edge constraint.
 * @deprecated This class is deprecated, as it is meant for EdgeTable implemented as a vector of hash tables.
 */
class EdgeConstraint
{
public:
  int from; /**< The starting location of the edge. */
  int to;   /**< The ending location of the edge. */
  int t;    /**< The time at which the edge is constrained. */

  /**
   * @brief Constructs an EdgeConstraint object.
   *
   * @param from_ The starting location of the edge.
   * @param to_ The ending location of the edge.
   * @param t_ The time at which the edge is constrained.
   */
  EdgeConstraint(int from_, int to_, int t_) : from(from_), to(to_), t(t_)
  {
  }

  /**
   * @brief Compares two EdgeConstraint objects for equality.
   *
   * @param other The other EdgeConstraint object to compare with.
   *
   * @return  True if the two EdgeConstraint objects are equal, false otherwise.
   */
  auto operator==(const EdgeConstraint& other) const -> bool;
};

/**
 * @brief A class representing a table of edge constraints.
 * @todo Merge the EdgeConstraintTable class with the EdgeConstraintTableWithAgentNums used in ConstraintTable.
 */
class EdgeConstraintTable
{
public:
  /**
   * @brief Constructs an EdgeConstraintTable object.
   *
   * @param instance_ The instance of the problem.
   */
  explicit EdgeConstraintTable(const Instance& instance_);

  /**
   * @brief Adds an edge constraint to the table.
   *
   * @param loc1 The starting location of the edge.
   * @param loc2 The ending location of the edge.
   * @param time The time at which the edge is constrained.
   */
  void add(int loc1, int loc2, int time);

  /**
   * @brief Removes an edge constraint from the table.
   *
   * @param loc1 The starting location of the edge.
   * @param loc2 The ending location of the edge.
   * @param time The time at which the edge is constrained.
   */
  void remove(int loc1, int loc2, int time);
  // auto get(int loc1, int loc2, int time) const -> bool;

  /**
   * @brief Checks if an edge constraint exists in the table.
   *
   * @param loc1 The starting location of the edge.
   * @param loc2 The ending location of the edge.
   * @param time The time at which the edge is constrained.
   *
   * @return True if the edge constraint exists, false otherwise.
   */
  [[nodiscard]] auto get(int loc1, int loc2, int time) const -> bool
  {
    const int dir = static_cast<int>(find_direction(loc1, loc2)) - 1;  // dont consider NONE direction
    assertm(dir >= 0 && dir < NUM_DIRECTIONS, "Invalid direction.");
    int loc2_free = instance.location_to_free_location(loc2);
    // return edge_constraints[dir][loc2].find(time) != edge_constraints[dir][loc2].end(); // unordered set implementation
    return std::find(edge_constraints[dir][loc2_free].begin(), edge_constraints[dir][loc2_free].end(), time) !=
           edge_constraints[dir][loc2_free].end();  // vector implementation
  }

  /**
   * @brief Resets the edge constraint table.
   */
  void reset()
  {
    edge_constraints.clear();
    edge_constraints.resize(NUM_DIRECTIONS);
    for (int i = 0; i < NUM_DIRECTIONS; i++)
    {
      edge_constraints[i].resize(instance.get_num_free_cells());
    }
  }

private:
  // std::unordered_set<EdgeConstraint, EdgeConstraint::hash_edge_collision> edge_constraints;  // constraints for edges
  // std::vector<std::vector<std::unordered_set<int>>> edge_constraints; // unordered set implementation
  std::vector<std::vector<std::vector<int>>> edge_constraints; /**< The edge constraints. */
  const Instance&                            instance;         /**< The instance of the problem. */
};

/**
 * @brief The SafeIntervalTable class stores the safe intervals for each location in the grid.
 */
class SafeIntervalTable
{
public:
  /**
   * @brief Constructs a SafeIntervalTable object.
   *
   * @param instance_ The instance of the problem.
   */
  explicit SafeIntervalTable(const Instance& instance_);

  EdgeConstraintTable edge_constraint_table; /**< The edge constraint table, which stores the edge collisions. */

#ifdef SIT_PARALLELIZATION
  /**
   * @brief Adds constraints to the safe interval table in parallel.
   *
   * @param path The path to add constraints for.
   * @warning This function is not properly tested and has known issues.
   */
  void add_constraints_parallel(const TimePointPath& path);

  /**
   * @brief Removes constraints from the safe interval table in parallel.
   *
   * @param path The path to remove constraints for.
   * @warning This function is not properly tested and has known issues.
   */
  void remove_constraints_parallel(const TimePointPath& path);
#endif

  /**
   * @brief Adds a constraint to the safe interval table.
   *
   * @param timepoint The time point to add as a constraint.
   */
  void add_constraint(const TimePoint& timepoint);

  /**
   * @brief Adds all constraints created by a path to the safe interval table.
   *
   * @param path The path to add constraints for.
   */
  void add_constraints(const TimePointPath& path);

  /**
   * @brief Removes a constraint from the safe interval table.
   *
   * @param timepoint The time point to remove as a constraint.
   */
  void remove_constraint(const TimePoint& timepoint);

  /**
   * @brief Removes all constraints created by a path from the safe interval table.
   *
   * @param path The path to remove constraints for.
   */
  void remove_constraints(const TimePointPath& path);

  /**
   * @brief Gets the safe intervals for a given location and a time interval.
   *
   * @param location The location to get the safe intervals for.
   * @param time_interval The time interal, the location can be entered. All returned intervals must have nonzero intersection with it.
   *
   * @return A pair of iterators representing the range of safe intervals for the given location and time interval.
   * @warning The returned safe intervals must be checked for edge collisions.
   */
  [[nodiscard]] auto get_safe_intervals(int location, const TimeInterval& time_interval) const
      -> std::pair<std::list<TimeInterval>::const_iterator, std::list<TimeInterval>::const_iterator>;

  /**
   * @brief Gets the first safe interval for a given location.
   *
   * @param location The location to get the first safe interval for.
   *
   * @return  An iterator to the first safe interval for the given location.
   */
  [[nodiscard]] auto get_first_safe_interval(int location) const -> std::list<TimeInterval>::const_iterator;

  /**
   * @brief Calculates the estimate of the maximum path length.
   *
   * @return  The estimate of the maximum path length.
   */
  [[nodiscard]] auto get_max_path_len_estimate() -> int;

  /**
   * @brief Updates the end of the latest constraint. Call after removing constraints, which might make the estimate invalid.
   */
  void update_latest_constraint_end_estimate();

  /**
   * @brief Builds the safe interval table sequentially.
   *
   * @param paths The paths to build the safe interval table for.
   */
  void build_sequential(const std::vector<TimePointPath>& paths);

  /**
   * @brief Get the minimal reach time of a location.
   *
   * @param goal The goal location to get the minimal reach time for.
   *
   * @return  The minimal reach time of the goal location.
   */
  [[nodiscard]] auto get_min_reach_time(int goal) const -> int
  {
    assertm(instance.get_map_data().is_in(goal), "Invalid goal location.");
    return safe_intervals[instance.location_to_free_location(goal)].back().t_min;
  }

  /**
   * @brief Gets the last safe interval for a given location.
   *
   * @param location The location to get the last safe interval for.
   *
   * @return A reference to the last safe interval for the given location.
   */
  [[nodiscard]] auto get_last_safe_interval(int location) const -> const TimeInterval&
  {
    assertm(instance.get_map_data().is_in(location), "Invalid location");
    return safe_intervals[instance.location_to_free_location(location)].back();
  }

  /**
   * @brief Resets the safe interval table.
   */
  void reset()
  {
    // reset safe intervals
    int num_free_cells = static_cast<int>(safe_intervals.size());
    safe_intervals     = std::vector<std::list<TimeInterval>>(num_free_cells, std::list<TimeInterval>(1, TimeInterval(0, INT_MAX)));

    // reset latest constraint end
    latest_constraint_end         = 0;
    latest_constraint_end_updated = true;

    // reset unlimited safe intervals
    unlimited_safe_intervals = instance.get_map_data().get_num_free_cells();

    // reset max path len estimate
    max_path_len_estimate = INT_MAX;

    // reset edge constraint table
    edge_constraint_table.reset();
  }

private:
  std::vector<std::list<TimeInterval>> safe_intervals;                  /**< safe intervals for each position */
  const Instance&                      instance;                        /**< The instance of the problem. */
  int                                  max_path_len_estimate = INT_MAX; /**< The estimate of the maximum path length. */
  int  unlimited_safe_intervals;             /**< The number of unlimited safe intervals used for the max_path_len_estimate calculation. */
  int  latest_constraint_end         = 0;    /**< The end of the latest constraint. */
  bool latest_constraint_end_updated = true; /**< Indicates if the latest constraint end has been updated. */
#ifdef SIT_PARALLELIZATION
  omp_locks locks; /**< The locks used for parallelization. */
#endif
};
