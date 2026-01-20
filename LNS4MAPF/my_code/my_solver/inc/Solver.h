/**
 * @file
 * @brief Contains solver and solution related classes and functions.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 08-01-2025
 */

#pragma once
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "Instance.h"
#include "utils.h"

/**
 * @brief A class that represents the solution of the MAPF problem.
 */
class Solution
{
public:
  bool feasible = true; /**< Indicates if the solution is feasible. */

  std::vector<TimePointPath> paths;           /**< Trajectories of the agents. */
  mutable std::vector<Path>  converted_paths; /**< Trajectories converted to paths. */
  std::vector<int>           delays;          /**< Delays of the agents. */
  std::vector<int>           destroyed_paths; /**< Destroyed paths */
  std::vector<int>           priorities;      /**< Priorities of the agents. */

  int cost;          /**< Cost of the solution. */
  int sum_of_delays; /**< Sum of delays calculated as sum of cost - distance of each path. */
  int sum_of_costs;  /**< Sum of costs of all path. */
  int makespan;      /**< Makespan of the solution. */

  /**
   * @brief Check, whether a solutiio is valid for a given instance.
   *
   * @param instance The instance to check the solution against.
   *
   * @return True if the solution is valid, false otherwise.
   */
  [[nodiscard]] auto is_valid(const Instance& instance) const -> bool;

  /**
   * @brief Function to load a solution from a file.
   *
   * @param filename The name of the file to load the solution from.
   * @param instance The instance the solution solves.
   */
  void load(const std::string& filename, const Instance& instance);

  /**
   * @brief Function to save a solution to a file. Uses the format from MAPF-LNS2 for compatibility.
   *
   * @param filename The name of the file to save the solution to.
   * @param instance The instance the solution solves.
   */
  void save(const std::string& filename, const Instance& instance) const;

  /**
   * @brief Calculate the cost of the solution.
   *
   * @param instance The instance the solution solves.
   */
  void calculate_cost(const Instance& instance);

  /**
   * @brief Convert the trajectories of the agents to paths.
   */
  void convert_paths() const;
};

/**
 * @brief A class that represents a solver for the MAPF problem.
 */
class Solver
{
public:
  const std::string name;     /**< Name of the solver. */
  const Instance&   instance; /**< Instance the solver solves. */

  /**
   * @brief Constructor for the Solver class.
   *
   * @param name_ Name of the solver.
   * @param instance_ Instance the solver solves.
   * @param rnd_generator_ Random number generator.
   */
  Solver(std::string name_, const Instance& instance_, std::mt19937& rnd_generator_)
      : name(std::move(name_)), instance(instance_), rnd_generator(rnd_generator_)
  {
  }

  /**
   * @brief Virtual Destructor for the Solver class.
   */
  virtual ~Solver() = default;

  /**
   * @brief Pure virtual function to solve the MAPF problem. Will be overridden by derived classes.
   */
  virtual void solve() = 0;

  Solution      solution;      /**< the so far best solution */
  std::mt19937& rnd_generator; /**< Random number generator. */
};
