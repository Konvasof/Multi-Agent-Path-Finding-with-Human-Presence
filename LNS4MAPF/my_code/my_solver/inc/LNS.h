/**
 * @file
 * @brief Implementation of Large Neighborhood Search, related classes and functions.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 08-01-2025
 */

#pragma once
#include <random>
#include <set>
#include <utility>

#include "ConstraintTable.h"
#include "SIPP.h"
#include "SharedData.h"
#include "Solver.h"

/**
 * @brief Enumeration of destroy strategies used in LNS.
 */
enum class DESTROY_TYPE
{
  RANDOM,
  RANDOMWALK,
  INTERSECTION,
  ADAPTIVE,
  RANDOM_CHOOSE,
  BLOCKED
};

/**
 * @brief Settings of the destroy operator.
 */
class Destroy_settings
{
public:
  /**
   * @brief Constructor for destroy settings.
   *
   * @param type_ Type of destroy strategy.
   * @param size_ Size of the destroy neighborhood.
   */
  Destroy_settings(DESTROY_TYPE type_, int size_) : type(type_), size(size_)
  {
  }
  DESTROY_TYPE type; /**< Type of destroy strategy. */
  int          size; /**< Size of the destroy neighborhood. */
};

/**
 * @brief Settings for the Large Neighborhood Search (LNS) algorithm.
 */
class LNS_settings
{
public:
  /**
   * @brief Constructor for LNS settings.
   *
   * @param max_iter_ Maximum number of iterations.
   * @param time_limit_ Time limit for the algorithm.
   * @param destroy_settings_ Settings for the destroy operator.
   * @param sipp_settings_ Settings for the SIPP algorithm.
   * @param restarts_ Whether to use restarts.
   */
  LNS_settings(int max_iter_, double time_limit_, Destroy_settings destroy_settings_, SIPP_settings sipp_settings_, bool restarts_ = false)
      : max_iter(max_iter_),
        time_limit(time_limit_),
        destroy_settings(destroy_settings_),
        sipp_settings(sipp_settings_),
        restarts(restarts_)
  {
  }
  int              max_iter;         /**< Maximum number of iterations. */
  double           time_limit;       /**< Time limit for the algorithm. */
  Destroy_settings destroy_settings; /**< Settings for the destroy operator. */
  SIPP_settings    sipp_settings;    /**< Settings for the SIPP algorithm. */
  bool             restarts;         /**< Whether to use restarts. */
};

/**
 * @brief Class representing a generic operator for the LNS algorithm.
 */
class Operator
{
public:
  /**
   * @brief Constructor for the operator.
   *
   * @param func_ Function to be executed by the operator.
   */
  explicit Operator(std::function<void(Solution& sol)> func_) : func(std::move(func_))
  {
  }

  /**
   * @brief Applies the operator to a given solution.
   *
   * @param solution The solution to be modified.
   */
  void apply(Solution& solution)
  {
    func(solution);
  }

private:
  const std::function<void(Solution& sol)> func; /**< Function to be executed by the operator. */
};

/**
 * @brief This class is used to log the iterations of the LNS algorithm.
 */
struct Logger
{
  /**
   * @brief Gets the string, which contains the names of the used operators.
   *
   * @return A vector of strings representing the names of the used operators.
   */
  [[nodiscard]] auto get_used_operator_str() const -> std::vector<std::string>
  {
    std::vector<std::string> used_operator_str;
    for (const auto& it : used_operator)
    {
      // convert the enum to string
      used_operator_str.emplace_back(magic_enum::enum_name(it));
    }
    return used_operator_str;
  }

  std::vector<DESTROY_TYPE> used_operator;       /**< Vector of used destroy operators. */
  std::vector<int>          bsf_solution_cost;   /**< Vector of best solution costs. */
  std::vector<int>          bsf_makespan;        /**< Vector of best makespan values. */
  std::vector<double>       iteration_time_cpu;  /**< Vector of CPU times for each iteration. */
  std::vector<double>       iteration_time_wall; /**< Vector of wall times for each iteration. */
};

/**
 * @brief Class representing the Large Neighborhood Search (LNS) algorithm.
 */
class LNS : public Solver
{
public:
  mutable std::vector<SIPPInfo> sipp_info; /**< Vector of sipp information for each lns iteration. */
  std::unique_ptr<SIPP>         planner;   /**< Pointer to the SIPP planner. */

  /**
   * @brief Constructor for the LNS class.
   *
   * @param instance_ The instance to be solved.
   * @param rnd_generator_ Random number generator.
   * @param shared_data_ Pointer to shared data.
   * @param settings_ Settings for the LNS algorithm.
   */
  LNS(const Instance& instance_, std::mt19937& rnd_generator_, SharedData* shared_data_, LNS_settings& settings_);

  /**
   * @brief Destructor for the LNS class.
   */
  ~LNS() override = default;

  /**
   * @brief Solves the given instance using the LNS algorithm.
   */


  /**
   * @brief Finds the initial solution for the LNS algorithm.
   *
   * @return  True if the initial solution is found, false otherwise.
   */
  auto find_initial_solution() -> bool;

  /**
   * @brief Runs the Prioritized Planning algorithm.
   *
   * @return The solution found by the algorithm.
   */
  auto PrioritizedPlanning() -> Solution;

  /**
   * @brief Getter for the number of generated nodes.
   *
   * @return The number of generated nodes.
   */
  [[nodiscard]] auto get_num_of_generated_nodes() const -> int
  {
    return planner->generated_all_iter;
  }

  /**
   * @brief Getter for the number of expanded nodes.
   *
   * @return The number of expanded nodes.
   */
  [[nodiscard]] auto get_num_of_expanded_nodes() const -> int
  {
    return planner->expanded_all_iter;
  }

  /**
   * @brief Discards the solution if it is not better than the previous one. Restores the SafeIntervalTable and ConstraintTable to its
   * previous state.
   *
   * @param sol The solution to be discarded.
   * @param prev_sol The previous solution.
   */
  void discard_solution(const Solution& sol, const Solution& prev_sol) const;

  /**
   * @brief Builds the constraint table for the given paths.
   *
   * @param paths The paths to be used for initializing the constraint table.
   */
  void initialize_constraint_table(const std::vector<TimePointPath>& paths);

  Operator                        destroy_operator;               /**< Operator for the destroy phase. */
  mutable std::unordered_set<int> already_planned;                /**< Set of already planned agents. */
  Logger                          log;                            /**< Logger for the LNS algorithm. */
  LNS_settings&                   settings;                       /**< Settings for the LNS algorithm. */
  bool                            found_initial_solution = false; /**< Flag indicating if the initial solution was found. */

  bool safety_aware_mode = false;
  std::vector<int> human_path_locations; // Cesta člověka (převedená na location ID)
  int safety_exit_location = -1;         // Cíl (dveře)

  bool validate_safety(const Solution& sol);

  void solve() override;
  void print_safety_report(); 
  int human_start_location = -1;
  std::vector<int> find_shortest_path(int start_loc, int goal_loc);
  bool check_reachability(int start_loc, int goal_loc, int start_time);

private:
  /**
   * @brief The random destroy operator function, which destroys random agents.
   *
   * @param sol The solution to be modified.
   */
  void destroy_random(Solution& sol) const;

  /**
   * @brief The random walk destroy operator function.
   *
   * @param sol The solution to be modified.
   */
  void destroy_randomwalk(Solution& sol) const;

  /**
   * @brief The random_walk function of the random walk destroy operator.
   *
   * @param agent_num The number of the current agent.
   * @param neighborhood_size The size of the neighborhood.
   * @param start_location The starting location of the randomwalk.
   * @param start_time  The starting time of the randomwalk.
   * @param upperbound The upper bound used in randomwalk.
   * @param chosen The set of already chosen agents.
   *
   * @return True, if the neighborhood is filled, false otherwise.
   */
  auto random_walk(int agent_num, int neighborhood_size, int start_location, int start_time, int upperbound, std::set<int>& chosen) const
      -> bool;

  /**
   * @brief The intersection destroy operator function.
   *
   * @param sol The solution to be modified.
   */
  void destroy_intersection(Solution& sol) const;

  /**
   * @brief The blocked destroy operator function.
   *
   * @param sol The solution to be modified.
   */
  void destroy_blocked(Solution& sol) const;

  /**
   * @brief The adaptive destroy operator function.
   *
   * @param sol The solution to be modified.
   */
  void destroy_adaptive(Solution& sol) const;

  /**
   * @brief The random choose destroy operator function.
   *
   * @param sol The solution to be modified.
   */
  void destroy_random_choose(Solution& sol) const;

  /**
   * @brief A function used in the intersection destroy operator to get the agents that are intersecting at a given location.
   *
   * @param neighborhood The set of agents in the neighborhood.
   * @param current The location, for which the intersection agents are added to the neighborhood.
   */
  void get_intersection_agents(std::unordered_set<int>& neighborhood, int current) const;

  /**
   * @brief The repair operator function, which repairs the solution.
   *
   * @param sol The solution to be repaired.
   */
  void repair_default(Solution& sol) const;

  SharedData*                 shared_data;                          /**< Pointer to shared data. */
  int                         iteration_num = 0;                    /**< Current iteration number. */
  double                      initial_solution_time;                /**< Time taken to find the initial solution. */
  double                      curr_time;                            /**< Current time. */
  Operator                    repair_operator;                      /**< The repair operator. */
  mutable ConstraintTable     constraint_table;                     /**< Constraint table used, which stores the dynamic constraints. */
  mutable bool                constraint_table_initialized = false; /**< Flag indicating if the constraint table is initialized. */
  mutable std::vector<double> destroy_weights;                      /**< Vector of weights for the adaptive destroy operator. */
  double                      reaction_factor = 0.01;               /**< Reaction factor for the adaptive destroy operator. */
  double                      decay_factor    = 0.01;               /**< Decay factor for the adaptive destroy operator. */
  mutable DESTROY_TYPE        last_destroy_strategy;                /**< Last used destroy strategy. */
  float                       threshold_blocked = 1.0;              /**< Threshold for the blocked destroy operator. */
};
