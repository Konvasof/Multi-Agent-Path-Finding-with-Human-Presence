/**
 * @file
 * @brief Contains classes and functions related to the SIPP pathfinding algorithm.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 08-02-2025
 */
#pragma once
#include <utils.h>

#include <random>

// #include <boost/heap/pairing_heap.hpp>
// #include <boost/heap/d_ary_heap.hpp>
#include <boost/heap/priority_queue.hpp>
// #include <queue>
// #include <unordered_map>
// #include <unordered_set>
// #include <boost/heap/fibonacci_heap.hpp>

#include "IterInfo.h"
#include "SafeIntervalTable.h"

/**
 * @brief Represents settings for the SIPP algorithm.
 */
class SIPP_settings
{
public:
  /**
   * @brief Constructor for SIPP_settings.
   *
   * @param implementation_ The implementation which should be used.
   * @param info_type_ The type of information, that should be generated.
   * @param w_ The suboptimality factor.
   * @param p_ The parameter p for the Bounded Suboptimal SIPP algorithm.
   */
  SIPP_settings(SIPP_implementation implementation_, INFO_type info_type_, double w_, int p_ = 1.0)
      : implementation(implementation_), info_type(info_type_), w(w_), p(p_)
  {
  }

  SIPP_implementation implementation; /**< The implementation which should be used. */
  INFO_type           info_type;      /**< The type of information, that should be generated during the search. */
  double              w;              /**< The suboptimality factor. */
  double              p;              /**< The parameter p for the Bounded Suboptimal SIPP algorithm. */
  // bool                generate_blocked = false;
};

/**
 * @brief Namespace for SIPP nodes.
 */
namespace sipp
{

/**
 * @brief Represents a node in the SIPP algorithm.
 */
class SIPPNode : public Node
{
public:
  TimePoint       time_point; /**< The time point (@see TimePoint) of the node. */
  SIPPNode const* parent;     /**< The parent node. */
  double          h2;         /**< The heuristic value for the second heuristic. */
  double          h3;         /**< The heuristic value for the third heuristic. */


  /**
   * @brief Constructor for SIPPNode.
   *
   * @param location The location of the node.
   * @param interval The time interval of the node.
   * @param g_ The g-value of the node.
   * @param h_ The primary heuristic value of the node.
   * @param h2_ The secondary heuristic value.
   * @param h3_ The tertiary heuristic value.
   * @param parent_ The parent node.
   */
  SIPPNode(int location, const TimeInterval& interval, double g_, double h_, double h2_, double h3_, SIPPNode const* parent_)
      : Node(g_, h_), time_point(location, interval), parent(parent_), h2(h2_), h3(h3_)
  {
  }

  /**
   * @brief Constructor for SIPPNode.
   *
   * @param time_point_ The time point of the node.
   * @param g_ The g-value of the node.
   * @param h_ The primary heuristic value of the node.
   * @param h2_ The secondary heuristic value.
   * @param h3_ The tertiary heuristic value.
   * @param parent_ The parent node.
   */
  SIPPNode(const TimePoint& time_point_, double g_, double h_, double h2_, double h3_, SIPPNode const* parent_)
      : Node(g_, h_), time_point(time_point_), parent(parent_), h2(h2_), h3(h3_)
  {
  }

  /**
   * @brief Equality operator for SIPPNode.
   *
   * @param other The other SIPPNode to compare with.
   *
   * @return True if the nodes are equal, false otherwise.
   */
  auto operator==(const SIPPNode& other) const -> bool;
};

/**
 * @brief Comparator class for SIPPNodes
 */
class SIPPNodeComparator
{
public:
  std::mt19937*                       rnd_generator; /**< Random number generator. */
  mutable std::bernoulli_distribution dist;          /**< Bernoulli distribution for generating random true/false values. */

  /**
   * @brief Constructor for SIPPNodeComparator.
   *
   * @param rnd_generator_ Pointer to a random number generator.
   */
  explicit SIPPNodeComparator(std::mt19937* rnd_generator_) : rnd_generator(rnd_generator_), dist(0.5)
  {
  }

  /**
   * @brief Comparison operator for SIPPNode.
   *
   * @param n1 The first SIPPNode to compare.
   * @param n2 The second SIPPNode to compare.
   *
   * @return True if n1 is less than n2, false otherwise.
   */
  auto operator()(SIPPNode const* n1, SIPPNode const* n2) const -> bool;
};


/**
 * @brief Comparator class for Bounded Suboptimal SIPP
 */
class SIPPNodeComparatorSuboptimal
{
public:
  std::mt19937*                       rnd_generator;              /**< Random number generator. */
  mutable std::bernoulli_distribution dist;                       /**< Bernoulli distribution for generating random true/false values. */
  int                                 max_suboptimality_absolute; /**< Maximum absolute suboptimality. */

  /**
   * @brief Constructor for SIPPNodeComparatorSuboptimal.
   *
   * @param rnd_generator_ Pointer to a random number generator.
   * @param max_suboptimality_absolute_ Maximum absolute suboptimality.
   */
  SIPPNodeComparatorSuboptimal(std::mt19937* rnd_generator_, int max_suboptimality_absolute_)
      : rnd_generator(rnd_generator_), dist(0.5), max_suboptimality_absolute(max_suboptimality_absolute_)
  {
  }

  /**
   * @brief Comparison operator for SIPPNode.
   *
   * @param n1 The first SIPPNode to compare.
   * @param n2 The second SIPPNode to compare.
   *
   * @return True if n1 is less than n2, false otherwise.
   */
  auto operator()(SIPPNode const* n1, SIPPNode const* n2) const -> bool;
};

/**
 * @brief Represents a pool of SIPP nodes.
 * This class manages the allocation and deallocation of SIPP nodes. Saves a lot of time. If there are not enough nodes in the pool, it
 * allocates more and stores them in a list, which is then merged into the pool at the end of the iteration. Once a node is allocated, it is
 * not deallocated until the pool is destroyed.
 */
class NodePool
{
public:
  /**
   * @brief Constructor for NodePool.
   *
   * @param size The initial size of the node pool.
   */
  explicit NodePool(int size);

  /**
   * @brief Add a node to the pool.
   *
   * @param node The SIPPNode to add to the pool.
   *
   * @return Pointer to the added SIPPNode.
   */
  auto add_node(SIPPNode node) -> SIPPNode*;

  /**
   * @brief Merge the extra nodes into the pool. Call this after a search algorithm is completed, to carry over the extra nodes to the next
   * run.
   */
  void merge_extra();

private:
  int                   end = 0; /**< The end index of the pool. Nodes after end are empty. */
  std::vector<SIPPNode> pool;    /**< The pool of SIPP nodes. */
  std::list<SIPPNode>   extra;   /**< The list of extra SIPP nodes. */
};


// using PriorityQueue = std::priority_queue<const SIPPNode*, std::vector<const SIPPNode*>, SIPPNodeComparator>;

/**
 * @brief Priority queue for SIPP nodes.
 */
using PriorityQueue = boost::heap::priority_queue<const SIPPNode*, boost::heap::compare<SIPPNodeComparator>>;

// using PriorityQueue = boost::heap::d_ary_heap<const SIPPNode*, boost::heap::arity<2>, boost::heap::compare<SIPPNodeComparator>>;

/**
 * @brief Priority queue for suboptimal SIPP nodes.
 */
using PriorityQueueSuboptimal = boost::heap::priority_queue<const SIPPNode*, boost::heap::compare<SIPPNodeComparatorSuboptimal>>;

// using PriorityQueue = boost::heap::fibonacci_heap<const SIPPNode*, boost::heap::compare<SIPPNode::compare_node_ptrs>>;
// using PriorityQueue = boost::heap::pairing_heap<const SIPPNode*, boost::heap::compare<SIPPNode::compare_node_ptrs>>;
// typedef std::unordered_set<TimePoint, TimePoint::hash_time_point> UnorderedSet;
// typedef std::unordered_map<Point2d, int, Point2d::hash_point>     UnorderedMap;

/**
 * @brief Extracts the path from the final node to the start node.
 *
 * @param final_node The final node of the path.
 *
 * @return A vector of TimePoints representing the path from the start node to the final node.
 */
auto extract_path(SIPPNode const* final_node) -> TimePointPath;

}  // namespace sipp

/**
 * @brief Class representing the SIPP algorithm.
 */
class SIPP
{
public:
  SafeIntervalTable safe_interval_table;     /**< The safe interval table. */
  int               generated_all_iter  = 0; /**< The number of nodes generated in all iterations. */
  int               generated_this_iter = 0; /**< The number of nodes generated in the current iteration. */
  int               expanded_all_iter   = 0; /**< The number of nodes expanded in all iterations. */
  int               expanded_this_iter  = 0; /**< The number of nodes expanded in the current iteration. */
  int               iteration_num       = 0; /**< The current iteration number. */
  SIPPInfo          iter_info;               /**< The iteration information. */

  /**
   * @brief Constructor for SIPP.
   *
   * @param instance_ The instance of the problem.
   * @param rnd_generator_ The random number generator.
   * @param settings_ The settings for the SIPP algorithm.
   */
  SIPP(const Instance& instance_, std::mt19937& rnd_generator_, const SIPP_settings& settings_);

  /**
   * @brief   Destructor for SIPP.
   */
  ~SIPP() = default;

  /**
   * @brief Plans a path for the given agent number.
   *
   * @param agent_num The agent number to plan the path for.
   * @param already_planned A set of already planned agents.
   *
   * @return A TimePointPath representing the planned path for the agent.
   */
  auto plan(int agent_num, const std::unordered_set<int>& already_planned) -> TimePointPath;

  /**
   * @brief Plans a path for the given agent number using the multi-heuristic version of the SIPP algorithm.
   *
   * @param agent_num The agent number to plan the path for.
   *
   * @return A TimePointPath representing the planned path for the agent.
   */
  auto plan_sipp_mine(int agent_num) -> TimePointPath;

  /**
   * @brief Plans a path for the given agent number using the ap multi-heuristic version of the SIPP algorithm.
   *
   * @param agent_num The agent number to plan the path for.
   * @param already_planned A set of already planned agents.
   *
   * @return A TimePointPath representing the planned path for the agent.
   */
  auto plan_sipp_mine_ap(int agent_num, const std::unordered_set<int>& already_planned) -> TimePointPath;

  /**
   * @brief Plans a path for the given agent number using the original single-heuristic version of the SIPP algorithm from MAPF-LNS.
   *
   * @param agent_num The agent number to plan the path for.
   *
   * @return A TimePointPath representing the planned path for the agent.
   */
  auto plan_mapflns_heuristic(int agent_num) -> TimePointPath;

  /**
   * @brief Plans a path for the given agent number using the original Bounded Suboptimal SIPP algorithm.
   *
   * @param agent_num The agent number to plan the path for.
   * @param already_planned A set of already planned agents.
   * @param w The suboptimality factor.
   * @param ap Whether to use the ap version.
   *
   * @return A TimePointPath representing the planned path for the agent.
   */
  auto plan_suboptimal(int agent_num, const std::unordered_set<int>& already_planned, double w = 1.0, bool ap = false) -> TimePointPath;

  /**
   * @brief Reset the SIPP algorithm.
   */
  void reset()
  {
    safe_interval_table.reset();
  }

private:
  /**
   * @brief Initialize the information about the iteration. Reset the number of generated and expanded nodes and the iteration number.
   */
  void initialize_iter_info();

  /**
   * @brief Update the iteration information.
   *
   * @param expanded The expanded node.
   * @param last_iter Whether this is the last iteration. If so, the alltime numbers of generated and expanded nodes are updated.
   */
  void update_iter_info(const sipp::SIPPNode& expanded, bool last_iter);

  const Instance&      instance;      /**< The instance of the problem. */
  sipp::NodePool       node_pool;     /**< The pool of SIPP nodes. */
  std::vector<int>     known_max;     /**< The known maximum time for each node. */
  std::vector<int>     known_min;     /**< The known minimum time for each node. */
  std::mt19937&        rnd_generator; /**< The random number generator. */
  const SIPP_settings& settings;      /**< The settings for the SIPP algorithm. */

  // for destroy operator that takes into account which agents block other agents
  // std::vector<int> blocked_counts;
};
