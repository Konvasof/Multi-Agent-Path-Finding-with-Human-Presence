/**
 * @file
 * @brief Contains the Constraint table and related classes.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 14-02-2025
 */

#pragma once

#include <climits>
#include <iostream>
#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Instance.h"
#include "magic_enum/magic_enum.hpp"
#include "utils.h"

/**
 * @brief If this is defined, the solver is compiled with parallelized versions of the Constraint Table.
 */
#define CT_PARALLELIZATION

/**
 * @brief A class, which represents an edge constraint along with the number of agent, which causes it.
 * @deprecated This class is deprecated, as it is meant for EdgeTable implemented as a vector of hash tables.
 */
class EdgeConstraintWithAgentNum
{
public:
  int from;      /**< The starting location of the edge. */
  int to;        /**< The ending location of the edge. */
  int t;         /**< The time when the agent traverses the edge. */
  int agent_num; /**< The number of the agent, which causes the constraint. */

  /**
   * @brief Constructs an edge constraint with the given parameters.
   *
   * @param from_ The starting location of the edge.
   * @param to_ The ending location of the edge.
   * @param t_ The time when the agent traverses the edge.
   * @param agent_num_ The number of the agent, which causes the constraint.
   */
  EdgeConstraintWithAgentNum(int from_, int to_, int t_, int agent_num_) : from(from_), to(to_), t(t_), agent_num(agent_num_)
  {
  }

  /**
   * @brief Equality check operator for the edge constraint.
   *
   * @param other The other edge constraint to compare with.
   *
   * @return  True if the edge constraints are equal, false otherwise.
   */
  auto operator==(const EdgeConstraintWithAgentNum& other) const -> bool;
};

/**
 * @brief Represents a table of all edge constraints for all agents.
 * @todo Test properly the best data structure for storing the constraints. Possibilities are vector of hashtables, vector of sets, vector
 * of vectors
 */
class EdgeConstraintTableWithAgentNums
{
public:
  /**
   * @brief Constructs an edge constraint table.
   *
   * @param instance  The instance of the problem.
   */
  explicit EdgeConstraintTableWithAgentNums(const Instance& instance);

  /**
   * @brief Adds an edge constraint to the table.
   *
   * @param loc1 The starting location of the edge.
   * @param loc2 The ending location of the edge.
   * @param time The time when the agent traverses the edge.
   * @param agent_num The number of the agent, which causes the constraint.
   */
  void add(int loc1, int loc2, int time, int agent_num);

  /**
   * @brief Removes an edge constraint from the table.
   *
   * @param loc1 The starting location of the edge.
   * @param loc2 The ending location of the edge.
   * @param time The time when the agent traverses the edge.
   */
  void remove(int loc1, int loc2, int time);
  // auto get(int loc1, int loc2, int time) const -> bool;

  /**
   * @brief Retrieves an edge constraint from the table.
   *
   * @param loc1 The starting location of the edge.
   * @param loc2 The ending location of the edge.
   * @param time The time when the agent traverses the edge.
   *
   * @return The number of the agent, which causes the constraint. -1 is returned if the constraint does not exist.
   */
  [[nodiscard]] auto get(int loc1, int loc2, int time) const -> int;

private:
  // std::unordered_set<EdgeConstraint, EdgeConstraint::hash_edge_collision> edge_constraints;  // constraints for edges
  // std::vector<std::vector<std::unordered_set<int>>> edge_constraints; // unordered set implementation
  std::vector<std::vector<std::vector<std::pair<int, int>>>>
                  edge_constraints; /**< vector implementation @todo try set, or something with log complexity*/
  const Instance& instance;         /**< The instance of the problem. */
};

/**
 * @brief A class representing a constraint table, which stores all dynamic constraints.
 */
class ConstraintTable
{
public:
  /**
   * @brief Constructs a constraint table for the given instance.
   *
   * @param instance_ The instance of the problem.
   */
  explicit ConstraintTable(const Instance& instance_);

  EdgeConstraintTableWithAgentNums edge_constraint_table; /**< The edge constraint table. */

  /**
   * @brief Sequentially builds the constraint table from the given paths.
   *
   * @param paths The paths to build the constraint table from.
   */
  void build_sequential(const std::vector<TimePointPath>& paths);
#ifdef CT_PARALLELIZATION
  /**
   * @brief Parallelized version of the build_sequential function. Only parallelizes on the level of agents.
   *
   * @param paths The paths to build the constraint table from.
   * @warning This function is not properly tested and has known issues.
   */
  void build_parallel(const std::vector<TimePointPath>& paths);

  /**
   * @brief Parallelized version of the build_sequential function. Parallelizes on the level of paths.
   *
   * @param paths The paths to build the constraint table from.
   * @warning This function is not properly tested and has known issues.
   */
  void build_parallel_2(const std::vector<TimePointPath>& paths);

  /**
   * @brief Adds constraints to the constraint table in parallel.
   *
   * @param path The path to add constraints for.
   * @param agent_num The number of the agent to add constraints for.
   * @warning This function is not properly tested and has known issues.
   */
  void add_constraints_parallel(const TimePointPath& path, int agent_num);

  /**
   * @brief Removes constraints from the constraint table in parallel.
   *
   * @param path The path to remove constraints for.
   * @param agent_num The number of the agent to remove constraints for.
   * @warning This function is not properly tested and has known issues.
   */
  void remove_constraints_parallel(const TimePointPath& path, int agent_num);
#endif

  /**
   * @brief Adds a constraint to the constraint table for the given timepoint and agent number.
   *
   * @param timepoint The timepoint to add the constraint for.
   * @param agent_num The number of the agent to add the constraint for.
   */
  void add_constraint(const TimePoint& timepoint, int agent_num);

  /**
   * @brief Adds a path of a given agent to the constraint table.
   *
   * @param path The path to add constraints for.
   * @param agent_num The number of the agent to add constraints for.
   */
  void add_constraints(const TimePointPath& path, int agent_num);

  /**
   * @brief Removes a constraint from the constraint table for the given timepoint and agent number.
   *
   * @param timepoint The timepoint to remove the constraint for.
   * @param agent_num The number of the agent to remove the constraint for.
   */
  void remove_constraint(const TimePoint& timepoint, int agent_num);

  /**
   * @brief Removes a path of a given agent from the constraint table.
   *
   * @param path The path to remove constraints for.
   * @param agent_num The number of the agent to remove constraints for.
   */
  void remove_constraints(const TimePointPath& path, int agent_num);

  /**
   * @brief Returns the agents that are blocking going from a given location to another given location at the given time.
   *
   * @param from The starting location.
   * @param to  The ending location.
   * @param time The time when the location is blocked.
   *
   * @return A pair of integers, where the first integer represents a vertex conflict and the second integer represents an edge conflict.
   */
  [[nodiscard]] auto get_blocking_agent(int from, int to, int time) const -> std::pair<int, int>;

  /**
   * @brief Finds agents that block a certaion location at a certain time and later.
   *
   * @param location The location to check for blocking agents.
   * @param time_min The minimum time to check for blocking agents.
   *
   * @return  A vector of integers representing the agents that block the location sorted in descending order from the latest to the
   * earliest.
   */
  [[nodiscard]] auto get_blocking_agents(int location, int time_min) const -> std::vector<int>;

  /**
   * @brief Returns agents, that enter a given location, along with how many they enter it.
   *
   * @param location The location to check for agents.
   *
   * @return A map of integers representing the agents that enter the location and how many times they enter it.
   */
  [[nodiscard]] auto get_agents_counts(int location) const -> const std::unordered_map<int, int>&
  {
    assertm(instance.get_map_data().is_in(location), "Invalid location.");
    return agents_counts[instance.location_to_free_location(location)];
  }

  /**
   * @brief Returns agents, that enter a given free location, along with how many they enter it.
   *
   * @param free_location The free location to check for agents.
   *
   * @return A map of integers representing the agents that enter the free location and how many times they enter it.
   */
  [[nodiscard]] auto get_agents_counts_free(int free_location) const -> const std::unordered_map<int, int>&
  {
    assertm(instance.get_map_data().is_in(instance.free_location_to_location(free_location)), "Invalid location.");
    return agents_counts[free_location];
  }

  /**
   * @brief Returns the last constraint start time for a given location.
   *
   * @param location The location to check for the last constraint start time.
   *
   * @return The last constraint start time for the given location.
   */
  [[nodiscard]] auto get_last_constraint_start(int location) const -> int
  {
    return constraints[instance.location_to_free_location(location)].back().first.t_min;
  }

private:
  std::vector<std::list<std::pair<TimeInterval, int>>> constraints;   /**< constraints for each location */
  const Instance&                                      instance;      /**< The instance of the problem. */
  std::vector<std::unordered_map<int, int>>            agents_counts; /**< unordered map of times the agents visit each location */
#ifdef CT_PARALLELIZATION
  omp_locks locks; /**< Locks for parallelization. */
#endif
};
