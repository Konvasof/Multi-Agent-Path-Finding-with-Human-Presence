/*
 * Author: Jan Chleboun
 * Date: 09-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description: Implementation of the SIPP algorithm for path planning in a grid map. The algorithm is based on the A* search algorithm, but
 * searches also in the time dimension. To reduce dimensionality of the space, the time is discretized into intervals.
 */

#include "SIPP.h"

#include <climits>
#include <cmath>
#include <memory>
#include <random>

#include "utils.h"

#define NODE_POOL_MERGE_SIZE 500  // merge the extra list with the node pool vector only of there are more elements in the list
#define INITIAL_NODE_POOL_SIZE 2  // the multiple of map size to use as initial node pool size

using namespace sipp;

SIPP::SIPP(const Instance& instance_, std::mt19937& rnd_generator_, const SIPP_settings& settings_)
    : safe_interval_table(SafeIntervalTable(instance_)),
      instance(instance_),
      node_pool(NodePool(INITIAL_NODE_POOL_SIZE * instance.get_num_cells())),
      rnd_generator(rnd_generator_),
      settings(settings_)
{
  known_max.resize(instance.get_num_cells(), -1);
  known_min.resize(instance.get_num_cells(), -1);
  // TODO maybe only remember free cells?
}


auto SIPPNode::operator==(const SIPPNode& other) const -> bool
{
  return g == other.g && h == other.h && time_point == other.time_point;
}

auto SIPPNodeComparator::operator()(SIPPNode const* n1, SIPPNode const* n2) const -> bool
{
  // compare f
  if (n1->f == n2->f)
  {
    if (n1->h == n2->h)
    {  // tie in reach time
      if (n1->h2 == n2->h2)
      {  // tie in distance from goal
        if (n1->h3 == n2->h3)
        {
          // if tie in all heuristics, choose randomly
          return dist(*rnd_generator);
        }
        // if f is the same and h is the same and h2 is the same prefer locations that dont block goals of other agents
        return n1->h3 > n2->h3;
      }
      return n1->h2 > n2->h2;  // if f is the same and h is the same prefer nodes closer to goal
    }
    return n1->h > n2->h;  // if f is the same, prefer earlier intervals
  }
  return n1->f > n2->f;
}

auto SIPPNodeComparatorSuboptimal::operator()(SIPPNode const* n1, SIPPNode const* n2) const -> bool
{
  // compare f
  if (std::abs(n1->f - n2->f) <= max_suboptimality_absolute)
  {
    if (n1->h == n2->h)
    {  // tie
      if (n1->h2 == n2->h2)
      {
        if (n1->f == n2->f)
        {
          if (n1->h3 == n2->h3)
          {
            // if tie, choose randomly
            return dist(*rnd_generator);
          }
          return n1->h3 > n2->h3;
        }
        return n1->f > n2->f;
      }
      return n1->h2 > n2->h2;
    }
    return n1->h > n2->h;
  }
  return n1->f > n2->f;
}


NodePool::NodePool(const int size)
{
  // initialize the pool with dummy nodes
  pool.resize(size, SIPPNode(0, {0, 0}, 0, 0, 0, 0, nullptr));
  end = 0;
}

auto NodePool::add_node(SIPPNode node) -> SIPPNode*
{
  // check, whether there is still space in the node pool
  if (end < (int)pool.size())
  {
    // move to the end of the pool and increment the count
    pool[end++] = std::move(node);
    return &pool[end - 1];
  }

  // if there is no space in the vector, add to the list, which will be merged with the vector at the end of the iteration
  extra.push_back(node);
  return &extra.back();
}


void NodePool::merge_extra()
{
  if (extra.size() > NODE_POOL_MERGE_SIZE)
  {
    // merge the list with the vector
    // be careful, this invalidates all pointers to the nodes in the node pool
    pool.insert(pool.end(), std::move_iterator{extra.begin()}, std::move_iterator{extra.end()});
    extra.clear();
  }
  // reset the node counter
  end = 0;
}

auto SIPP::plan(int agent_num, const std::unordered_set<int>& already_planned) -> TimePointPath
{
  // reset vector of blocked counts
  // if (settings.generate_blocked)
  // {
  //   blocked_counts.resize(0, instance.get_num_of_agents());
  // }

  if (settings.implementation == SIPP_implementation::SIPP_suboptimal)
  {
    // suboptimal sipp
    return plan_suboptimal(agent_num, already_planned, settings.w, false);
  }

  if (settings.implementation == SIPP_implementation::SIPP_suboptimal_ap)
  {
    // suboptimal sipp
    return plan_suboptimal(agent_num, already_planned, settings.w, true);
  }

  assertm(settings.w == 1.0, "Suboptimality factor must be 1 for optimal algorithms.");
  // optimal sipp
  if (settings.implementation == SIPP_implementation::SIPP_mine)
  {
    return plan_sipp_mine(agent_num);
  }

  if (settings.implementation == SIPP_implementation::SIPP_mine_ap)
  {
    return plan_sipp_mine_ap(agent_num, already_planned);
  }

  if (settings.implementation == SIPP_implementation::SIPP_mapf_lns)
  {
    return plan_mapflns_heuristic(agent_num);
  }

  throw std::runtime_error("Unknown SIPP implementation");
}

auto SIPP::plan_sipp_mine(const int agent_num) -> TimePointPath
{
  if (settings.info_type != INFO_type::no_info)
  {
    initialize_iter_info();
  }

  const Map& map_data = instance.get_map_data();
  const int  start    = instance.get_start_locations()[agent_num];
  const int  goal     = instance.get_goal_locations()[agent_num];


  // check whether start and goal inside the map and not obstructed
  assertm(map_data.is_in(start) && map_data.is_in(goal), "Start or goal outside the map.");
  assertm(map_data.index(start) == 0 && map_data.index(goal) == 0, "Start or goal obstructed.");

  // estimate the minimum and maximum time the goal can be reached
  const int min_time = safe_interval_table.get_min_reach_time(goal);
  const int max_time = safe_interval_table.get_max_path_len_estimate();
  assertm(min_time >= 0 && max_time >= 0, "Time can not be negative.");

  // create openlist
  sipp::PriorityQueue open_list{SIPPNodeComparator(&rnd_generator)};

  // retrieve the first safe interval
  auto time_interval_start = safe_interval_table.get_first_safe_interval(start);

  // calculate the heuristic of the start node
  {
    const double h2 = instance.get_heuristic_distance(agent_num, start);
    const double h1 = std::max((double)min_time, h2);

    // create the start node
    open_list.push(node_pool.add_node(SIPPNode(start, *time_interval_start, 0, h1, h2, 0, nullptr)));

    if (settings.info_type != INFO_type::no_info)
    {
      generated_this_iter++;
    }
  }

  // clear known nodes database
  std::fill(known_min.begin(), known_min.end(), INT_MAX);
  std::fill(known_max.begin(), known_max.end(), -1);

  // main loop
  while (!open_list.empty())
  {
    if (settings.info_type != INFO_type::no_info)
    {
      iteration_num++;
    }

    // pop the node to be processed
    SIPPNode const* const current = open_list.top();
    open_list.pop();

    // std::cout << "\tCurrent: " << instance.location_to_position(current->time_point.location) << ", " << current->time_point.interval
    //          << ", f: " << current->f << std::endl;

    // check whether node was expanded already (same position and tmax higher then current tmin)
    if (current->time_point.interval.t_min >= known_min[current->time_point.location] &&
        current->time_point.interval.t_min <= known_max[current->time_point.location])
    {
      continue;
    }

    // check whether goal location is reached and that there will be no more dynamic constraints at the goal location
    if (current->time_point.location == goal && current->time_point.interval.t_max == INT_MAX)
    {
      // extract the path
      TimePointPath ret = sipp::extract_path(current);

      // reset the node pool
      node_pool.merge_extra();

      // generate last iteration info
      if (settings.info_type != INFO_type::no_info)
      {
        update_iter_info(*current, true);
      }

      return ret;
    }

    // add to known nodes
    known_min[current->time_point.location] = std::min(current->time_point.interval.t_min, known_min[current->time_point.location]);
    known_max[current->time_point.location] = std::max(current->time_point.interval.t_max, known_max[current->time_point.location]);
    // std::cout << "Known min: " << known_min[current->time_point.location] << std::endl;

    // calculate the time window to enter neighboring positions
    const TimeInterval neighbor_entry_time_interval(safe_increase(current->time_point.interval.t_min),
                                                    safe_increase(current->time_point.interval.t_max));

    // expand current node
    const std::vector<int>& neighbors = instance.get_neighbor_locations(current->time_point.location);
    for (const auto& neighbor : neighbors)
    {
      // calculate the heuristic
      double h2 = instance.get_heuristic_distance(agent_num, neighbor);

      double h3 = 0;
      // prefer non goal locations
      if (neighbor != goal && instance.is_goal_location(neighbor))
      {
        h3 = 1.0;  // this is the logically right option
                   // h2 += 1.0; // this works well but shouldnt
      }
      // std::cout << "Neighbor: " << instance.location_to_position(neighbor) << ", h2: " << h2 << std::endl;

      // retrieve the safe intervals for the neighbor

      auto [sf_start, sf_end] = safe_interval_table.get_safe_intervals(neighbor, neighbor_entry_time_interval);

      for (auto it = sf_start; it != sf_end; it++)
      {
        TimePoint neighbor_time_point(neighbor, *it);
        // modify the interval to be valid
        if (it->t_min < neighbor_entry_time_interval.t_min)
        {
          neighbor_time_point.interval.t_min = neighbor_entry_time_interval.t_min;
        }
        assertm(neighbor_time_point.interval.t_min <= neighbor_time_point.interval.t_max, "Modifying the time interval made it invalid.");

        // skip intervals that start later then the maximum time to reach the goal
        if (neighbor_time_point.interval.t_min > max_time)
        {
          continue;
        }

        // check edge collision
        if (safe_interval_table.edge_constraint_table.get(neighbor_time_point.location, current->time_point.location,
                                                          neighbor_time_point.interval.t_min))
        {
          continue;
        }

        assertm(neighbor_time_point.interval.t_min <= neighbor_entry_time_interval.t_max, "Unreachable interval among neighbors.");

        assertm(neighbor_time_point.interval.t_min <= min_time || known_min[neighbor] == INT_MAX ||
                    neighbor_time_point.interval.t_min >= known_min[neighbor],
                "The heuristic can be non consistent only for nodes starting before the goal obstruction time.");
        // check whether known
        if (neighbor_time_point.interval.t_min >= known_min[neighbor] && neighbor_time_point.interval.t_min <= known_max[neighbor])
        {
          continue;
        }


        assertm(neighbor_time_point.interval.t_min > current->time_point.interval.t_min, "Invalid interval.");

        // check whether the is goal reached in the minimal reach time
        // if (neighbor_time_point.location == goal && neighbor_time_point.interval.t_min == min_time)
        //{
        //  // add with heuristic INT_MIN so that the node is selected next
        //  open_list.push(node_pool.add_node(SIPPNode(neighbor_time_point, neighbor_time_point.interval.t_min, INT_MIN, INT_MIN,
        //  current)));
        //}
        // else
        //{
        //  // add to the open list
        //  open_list.push(node_pool.add_node(SIPPNode(neighbor_time_point, neighbor_time_point.interval.t_min, h, current)));
        //}
        const double h1 = std::max((double)min_time, neighbor_time_point.interval.t_min + h2) - neighbor_time_point.interval.t_min;
        open_list.push(node_pool.add_node(SIPPNode(neighbor_time_point, neighbor_time_point.interval.t_min, h1, h2, h3, current)));
        // std::cout << "interval:" << neighbor_time_point.interval << ", h1: " << h1 << ", h2: " << h2 << std::endl;

        if (settings.info_type != INFO_type::no_info)
        {
          generated_this_iter++;
        }
      }
    }

    // save the iteration info
    if (settings.info_type != INFO_type::no_info)
    {
      update_iter_info(*current, false);
    }
  }
  // reset the node pool
  node_pool.merge_extra();

  return TimePointPath();
}


auto SIPP::plan_sipp_mine_ap(const int agent_num, const std::unordered_set<int>& already_planned) -> TimePointPath
{
  assertm(already_planned.find(agent_num) == already_planned.end(), "Planning agent that was already planned.");

  if (settings.info_type != INFO_type::no_info)
  {
    initialize_iter_info();
  }

  const Map& map_data = instance.get_map_data();
  const int  start    = instance.get_start_locations()[agent_num];
  const int  goal     = instance.get_goal_locations()[agent_num];


  // check whether start and goal inside the map and not obstructed
  assertm(map_data.is_in(start) && map_data.is_in(goal), "Start or goal outside the map.");
  assertm(map_data.index(start) == 0 && map_data.index(goal) == 0, "Start or goal obstructed.");

  // estimate the minimum and maximum time the goal can be reached
  const int min_time = safe_interval_table.get_min_reach_time(goal);
  const int max_time = safe_interval_table.get_max_path_len_estimate();
  assertm(min_time >= 0 && max_time >= 0, "Time can not be negative.");

  // std::cout << "Agent: " << agent_num << ", min time: " << min_time << std::endl;
  // std::cout << "Min time estimate is: " << min_time << std::endl;
  // std::cout << "Max time estimate is: " << max_time << std::endl;


  // create openlist
  sipp::PriorityQueue open_list{SIPPNodeComparator(&rnd_generator)};

  // retrieve the first safe interval
  auto time_interval_start = safe_interval_table.get_first_safe_interval(start);

  // calculate the heuristic of the start node
  {
    const double h2 = instance.get_heuristic_distance(agent_num, start);
    const double h1 = std::max((double)min_time, h2);

    // create the start node
    open_list.push(node_pool.add_node(SIPPNode(start, *time_interval_start, 0, h1, h2, 0, nullptr)));

    if (settings.info_type != INFO_type::no_info)
    {
      generated_this_iter++;
    }
  }

  // clear known nodes database
  std::fill(known_min.begin(), known_min.end(), INT_MAX);
  std::fill(known_max.begin(), known_max.end(), -1);

  // main loop
  while (!open_list.empty())
  {
    if (settings.info_type != INFO_type::no_info)
    {
      iteration_num++;
    }

    // pop the node to be processed
    SIPPNode const* const current = open_list.top();
    open_list.pop();

    // std::cout << "\tCurrent: " << instance.location_to_position(current->time_point.location) << ", " << current->time_point.interval
    //          << ", f: " << current->f << std::endl;

    // check whether node was expanded already (same position and tmax higher then current tmin)
    if (current->time_point.interval.t_min >= known_min[current->time_point.location] &&
        current->time_point.interval.t_min <= known_max[current->time_point.location])
    {
      continue;
    }

    // check whether goal location is reached and that there will be no more dynamic constraints at the goal location
    if (current->time_point.location == goal && current->time_point.interval.t_max == INT_MAX)
    {
      // extract the path
      TimePointPath ret = sipp::extract_path(current);

      // reset the node pool
      node_pool.merge_extra();

      // generate last iteration info
      if (settings.info_type != INFO_type::no_info)
      {
        update_iter_info(*current, true);
      }

      return ret;
    }

    // add to known nodes
    known_min[current->time_point.location] = std::min(current->time_point.interval.t_min, known_min[current->time_point.location]);
    known_max[current->time_point.location] = std::max(current->time_point.interval.t_max, known_max[current->time_point.location]);
    // std::cout << "Known min: " << known_min[current->time_point.location] << std::endl;

    // calculate the time window to enter neighboring positions
    const TimeInterval neighbor_entry_time_interval(safe_increase(current->time_point.interval.t_min),
                                                    safe_increase(current->time_point.interval.t_max));

    // expand current node
    const std::vector<int>& neighbors = instance.get_neighbor_locations(current->time_point.location);
    for (const auto& neighbor : neighbors)
    {
      // calculate the heuristic
      double h_distance = instance.get_heuristic_distance(agent_num, neighbor);

      double h_ap_goal = 0;
      // penalize goal locations of not yet planned agents
      if (neighbor != goal && instance.is_goal_location(neighbor) &&
          already_planned.find(instance.whose_goal(neighbor)) == already_planned.end())
      {
        h_ap_goal = 1.0;
      }
      // std::cout << "Neighbor: " << instance.location_to_position(neighbor) << ", h2: " << h2 << std::endl;

      // retrieve the safe intervals for the neighbor

      auto [sf_start, sf_end] = safe_interval_table.get_safe_intervals(neighbor, neighbor_entry_time_interval);

      for (auto it = sf_start; it != sf_end; it++)
      {
        TimePoint neighbor_time_point(neighbor, *it);
        // modify the interval to be valid
        if (it->t_min < neighbor_entry_time_interval.t_min)
        {
          neighbor_time_point.interval.t_min = neighbor_entry_time_interval.t_min;
        }
        assertm(neighbor_time_point.interval.t_min <= neighbor_time_point.interval.t_max, "Modifying the time interval made it invalid.");

        // skip intervals that start later then the maximum time to reach the goal
        if (neighbor_time_point.interval.t_min > max_time)
        {
          continue;
        }

        // check edge collision
        if (safe_interval_table.edge_constraint_table.get(neighbor_time_point.location, current->time_point.location,
                                                          neighbor_time_point.interval.t_min))
        {
          continue;
        }

        assertm(neighbor_time_point.interval.t_min <= neighbor_entry_time_interval.t_max, "Unreachable interval among neighbors.");

        assertm(neighbor_time_point.interval.t_min <= min_time || known_min[neighbor] == INT_MAX ||
                    neighbor_time_point.interval.t_min >= known_min[neighbor],
                "The heuristic can be non consistent only for nodes starting before the goal obstruction time.");
        // check whether known
        if (neighbor_time_point.interval.t_min >= known_min[neighbor] && neighbor_time_point.interval.t_min <= known_max[neighbor])
        {
          continue;
        }


        assertm(neighbor_time_point.interval.t_min > current->time_point.interval.t_min, "Invalid interval.");

        // check whether the is goal reached in the minimal reach time
        // if (neighbor_time_point.location == goal && neighbor_time_point.interval.t_min == min_time)
        //{
        //  // add with heuristic INT_MIN so that the node is selected next
        //  open_list.push(node_pool.add_node(SIPPNode(neighbor_time_point, neighbor_time_point.interval.t_min, INT_MIN, INT_MIN,
        //  current)));
        //}
        // else
        //{
        //  // add to the open list
        //  open_list.push(node_pool.add_node(SIPPNode(neighbor_time_point, neighbor_time_point.interval.t_min, h, current)));
        //}
        const double h1 = std::max((double)min_time, neighbor_time_point.interval.t_min + h_distance) - neighbor_time_point.interval.t_min;
        open_list.push(
            node_pool.add_node(SIPPNode(neighbor_time_point, neighbor_time_point.interval.t_min, h1, h_ap_goal, h_distance, current)));
        // std::cout << "interval:" << neighbor_time_point.interval << ", h1: " << h1 << ", h2: " << h2 << std::endl;

        if (settings.info_type != INFO_type::no_info)
        {
          generated_this_iter++;
        }
      }
    }

    // save the iteration info
    if (settings.info_type != INFO_type::no_info)
    {
      update_iter_info(*current, false);
    }
  }
  // reset the node pool
  node_pool.merge_extra();

  return TimePointPath();
}


auto SIPP::plan_suboptimal(const int agent_num, const std::unordered_set<int>& already_planned, double w, bool ap) -> TimePointPath
{
  assertm(w >= 1.0, "Suboptimality factor must be more than 1");
  assertm(already_planned.find(agent_num) == already_planned.end(), "Planning agent that was already planned.");

  if (settings.info_type != INFO_type::no_info)
  {
    initialize_iter_info();
  }

  const Map& map_data = instance.get_map_data();
  const int  start    = instance.get_start_locations()[agent_num];
  const int  goal     = instance.get_goal_locations()[agent_num];


  // check whether start and goal inside the map and not obstructed
  assertm(map_data.is_in(start) && map_data.is_in(goal), "Start or goal outside the map.");
  assertm(map_data.index(start) == 0 && map_data.index(goal) == 0, "Start or goal obstructed.");

  // calculate the absolute suboptimality factor
  const int suboptimality_absolute = std::floor((w - 1.0) * instance.get_heuristic_distance(agent_num, start));
  assertm(w != 1.0 || suboptimality_absolute == 0, "Absolute suboptimality must be 0 when w is 1.0");

  // estimate the minimum and maximum time the goal can be reached
  const int min_time = safe_interval_table.get_min_reach_time(goal);
  // const int min_time = safe_interval_table.get_min_reach_time(goal) + suboptimality_absolute;
  const int max_time = safe_interval_table.get_max_path_len_estimate();
  assertm(min_time >= 0 && max_time >= 0, "Time can not be negative.");

  // create openlist
  sipp::PriorityQueueSuboptimal open_list{SIPPNodeComparatorSuboptimal(&rnd_generator, suboptimality_absolute)};

  // retrieve the first safe interval
  auto time_interval_start = safe_interval_table.get_first_safe_interval(start);

  // calculate the heuristic of the start node
  {
    const double h2 = instance.get_heuristic_distance(agent_num, start);
    const double h1 = std::max((double)min_time, h2);

    // create the start node
    open_list.push(node_pool.add_node(SIPPNode(start, *time_interval_start, 0, h1, h2, 0, nullptr)));

    if (settings.info_type != INFO_type::no_info)
    {
      generated_this_iter++;
    }
  }

  // clear known nodes database
  std::fill(known_min.begin(), known_min.end(), INT_MAX);
  std::fill(known_max.begin(), known_max.end(), -1);

  // main loop
  while (!open_list.empty())
  {
    if (settings.info_type != INFO_type::no_info)
    {
      iteration_num++;
    }

    // pop the node to be processed
    SIPPNode const* const current = open_list.top();
    open_list.pop();

    // std::cout << "\tCurrent: " << instance.location_to_position(current->time_point.location) << ", " << current->time_point.interval
    //          << ", f: " << current->f << std::endl;

    // check whether node was expanded already (same position and tmax higher then current tmin)
    if (current->time_point.interval.t_min >= known_min[current->time_point.location] &&
        current->time_point.interval.t_min <= known_max[current->time_point.location])
    {
      continue;
    }

    // check whether goal location is reached and that there will be no more dynamic constraints at the goal location
    if (current->time_point.location == goal && current->time_point.interval.t_max == INT_MAX)
    {
      // extract the path
      TimePointPath ret = sipp::extract_path(current);

      // reset the node pool
      node_pool.merge_extra();

      // generate last iteration info
      if (settings.info_type != INFO_type::no_info)
      {
        update_iter_info(*current, true);
      }

      return ret;
    }

    // add to known nodes
    known_min[current->time_point.location] = std::min(current->time_point.interval.t_min, known_min[current->time_point.location]);
    known_max[current->time_point.location] = std::max(current->time_point.interval.t_max, known_max[current->time_point.location]);
    // std::cout << "Known min: " << known_min[current->time_point.location] << std::endl;

    // calculate the time window to enter neighboring positions
    const TimeInterval neighbor_entry_time_interval(safe_increase(current->time_point.interval.t_min),
                                                    safe_increase(current->time_point.interval.t_max));

    // expand current node
    const std::vector<int>& neighbors = instance.get_neighbor_locations(current->time_point.location);
    for (const auto& neighbor : neighbors)
    {
      // calculate the heuristic
      // const double h = instance.get_heuristic_euclidean(agent_num, neighbor);
      // const double h = instance.get_heuristic_manhattan(agent_num, neighbor);
      double h2 = instance.get_heuristic_distance(agent_num, neighbor);
      // std::cout << "Neighbor: " << instance.location_to_position(neighbor) << ", h2: " << h2 << std::endl;

      double h3 = 0;
      if (ap)
      {
        if (neighbor != goal && instance.is_goal_location(neighbor) &&
            already_planned.find(instance.whose_goal(neighbor)) == already_planned.end())
        {
          h2 += settings.p;
        }
      }
      else
      {
        if (neighbor != goal && instance.is_goal_location(neighbor))
        {
          h2 += settings.p;
        }
      }
      // retrieve the safe intervals for the neighbor
      auto [sf_start, sf_end] = safe_interval_table.get_safe_intervals(neighbor, neighbor_entry_time_interval);

      for (auto it = sf_start; it != sf_end; it++)
      {
        TimePoint neighbor_time_point(neighbor, *it);
        // modify the interval to be valid
        if (it->t_min < neighbor_entry_time_interval.t_min)
        {
          neighbor_time_point.interval.t_min = neighbor_entry_time_interval.t_min;
        }
        assertm(neighbor_time_point.interval.t_min <= neighbor_time_point.interval.t_max, "Modifying the time interval made it invalid.");

        // skip intervals that start later then the maximum time to reach the goal
        if (neighbor_time_point.interval.t_min > max_time)
        {
          continue;
        }

        // check edge collision
        if (safe_interval_table.edge_constraint_table.get(neighbor_time_point.location, current->time_point.location,
                                                          neighbor_time_point.interval.t_min))
        {
          continue;
        }

        assertm(neighbor_time_point.interval.t_min <= neighbor_entry_time_interval.t_max, "Unreachable interval among neighbors.");

        // check whether known
        if (neighbor_time_point.interval.t_min >= known_min[neighbor] && neighbor_time_point.interval.t_min <= known_max[neighbor])
        {
          continue;
        }


        assertm(neighbor_time_point.interval.t_min > current->time_point.interval.t_min, "Invalid interval.");

        // check whether the is goal reached in the minimal reach time
        // if (neighbor_time_point.location == goal && neighbor_time_point.interval.t_min == min_time)
        //{
        //  // add with heuristic INT_MIN so that the node is selected next
        //  open_list.push(node_pool.add_node(SIPPNode(neighbor_time_point, neighbor_time_point.interval.t_min, INT_MIN, INT_MIN,
        //  current)));
        //}
        // else
        //{
        //  // add to the open list
        //  open_list.push(node_pool.add_node(SIPPNode(neighbor_time_point, neighbor_time_point.interval.t_min, h, current)));
        //}
        const double h1 = std::max((double)min_time, neighbor_time_point.interval.t_min + h2) - neighbor_time_point.interval.t_min;
        open_list.push(node_pool.add_node(SIPPNode(neighbor_time_point, neighbor_time_point.interval.t_min, h1, h2, h3, current)));
        // std::cout << "interval:" << neighbor_time_point.interval << ", h1: " << h1 << ", h2: " << h2 << std::endl;

        if (settings.info_type != INFO_type::no_info)
        {
          generated_this_iter++;
        }
      }
    }

    // save the iteration info
    if (settings.info_type != INFO_type::no_info)
    {
      update_iter_info(*current, false);
    }
  }
  // reset the node pool
  node_pool.merge_extra();

  return TimePointPath();
}


auto SIPP::plan_mapflns_heuristic(int agent_num) -> TimePointPath
{
  if (settings.info_type != INFO_type::no_info)
  {
    initialize_iter_info();
  }

  const Map& map_data = instance.get_map_data();
  const int  start    = instance.get_start_locations()[agent_num];
  const int  goal     = instance.get_goal_locations()[agent_num];

  // check whether start and goal inside the map and not obstructed
  assertm(map_data.is_in(start) && map_data.is_in(goal), "Start or goal outside the map.");
  assertm(map_data.index(start) == 0 && map_data.index(goal) == 0, "Start or goal obstructed.");

  // estimate the minimum and maximum time the goal can be reached
  const int min_time = safe_interval_table.get_min_reach_time(goal);
  const int max_time = safe_interval_table.get_max_path_len_estimate();
  assertm(min_time >= 0 && max_time >= 0, "Time can not be negative.");

  // create openlist
  sipp::PriorityQueue open_list{SIPPNodeComparator(&rnd_generator)};

  // retrieve the first safe interval
  auto time_interval_start = safe_interval_table.get_first_safe_interval(start);

  // calculate the heuristic of the start node
  {
    const double h = std::max((double)min_time, (double)instance.get_heuristic_distance(agent_num, start));

    // create the start node
    open_list.push(node_pool.add_node(SIPPNode(start, *time_interval_start, 0, h, h, 0, nullptr)));

    if (settings.info_type != INFO_type::no_info)
    {
      generated_this_iter++;
    }
  }

  // clear known nodes database
  std::fill(known_min.begin(), known_min.end(), INT_MAX);
  std::fill(known_max.begin(), known_max.end(), -1);

  // main loop
  while (!open_list.empty())
  {
    if (settings.info_type != INFO_type::no_info)
    {
      iteration_num++;
    }

    // pop the node to be processed
    SIPPNode const* const current = open_list.top();
    open_list.pop();

    // std::cout << "\tCurrent: " << instance.location_to_position(current->time_point.location) << ", " << current->time_point.interval
    //          << ", f: " << current->f << std::endl;

    // check whether node was expanded already (same position and tmax higher then current tmin)
    if (current->time_point.interval.t_min >= known_min[current->time_point.location] &&
        current->time_point.interval.t_min <= known_max[current->time_point.location])
    {
      continue;
    }

    // check whether goal location is reached and that there will be no more dynamic constraints at the goal location
    if (current->time_point.location == goal && current->time_point.interval.t_max == INT_MAX)
    {
      // extract the path
      TimePointPath ret = sipp::extract_path(current);

      // reset the node pool
      node_pool.merge_extra();

      // generate last iteration info
      if (settings.info_type != INFO_type::no_info)
      {
        update_iter_info(*current, true);
      }

      return ret;
    }

    // add to known nodes
    known_min[current->time_point.location] = std::min(current->time_point.interval.t_min, known_min[current->time_point.location]);
    known_max[current->time_point.location] = std::max(current->time_point.interval.t_max, known_max[current->time_point.location]);
    // std::cout << "Known min: " << known_min[current->time_point.location] << std::endl;

    // calculate the time window to enter neighboring positions
    const TimeInterval neighbor_entry_time_interval(safe_increase(current->time_point.interval.t_min),
                                                    safe_increase(current->time_point.interval.t_max));

    // expand current node
    const std::vector<int>& neighbors = instance.get_neighbor_locations(current->time_point.location);
    for (const auto& neighbor : neighbors)
    {
      // calculate the heuristic
      // const double h = instance.get_heuristic_euclidean(agent_num, neighbor);
      // const double h = instance.get_heuristic_manhattan(agent_num, neighbor);
      const double h2 = instance.get_heuristic_distance(agent_num, neighbor);
      // std::cout << "Neighbor: " << instance.location_to_position(neighbor) << ", h2: " << h2 << std::endl;

      // retrieve the safe intervals for the neighbor

      std::list<TimeInterval>::const_iterator sf_start, sf_end;
      std::tie(sf_start, sf_end) = safe_interval_table.get_safe_intervals(neighbor, neighbor_entry_time_interval);

      for (auto it = sf_start; it != sf_end; it++)
      {
        TimePoint neighbor_time_point(neighbor, *it);
        // modify the interval to be valid
        if (it->t_min < neighbor_entry_time_interval.t_min)
        {
          neighbor_time_point.interval.t_min = neighbor_entry_time_interval.t_min;
        }
        assertm(neighbor_time_point.interval.t_min <= neighbor_time_point.interval.t_max, "Modifying the time interval made it invalid.");

        // skip intervals that start later then the maximum time to reach the goal
        if (neighbor_time_point.interval.t_min > max_time)
        {
          continue;
        }

        // check edge collision
        if (safe_interval_table.edge_constraint_table.get(neighbor_time_point.location, current->time_point.location,
                                                          neighbor_time_point.interval.t_min))
        {
          continue;
        }

        assertm(neighbor_time_point.interval.t_min <= neighbor_entry_time_interval.t_max, "Unreachable interval among neighbors.");

        // assertm(neighbor_time_point.interval.t_min <= min_time || known_min[neighbor] == INT_MAX ||
        //             neighbor_time_point.interval.t_min >= known_min[neighbor],
        //         "The heuristic can be non consistent only for nodes starting before the goal obstruction time.");
        // check whether known
        if (neighbor_time_point.interval.t_min >= known_min[neighbor] && neighbor_time_point.interval.t_min <= known_max[neighbor])
        {
          continue;
        }


        assertm(neighbor_time_point.interval.t_min > current->time_point.interval.t_min, "Invalid interval.");

        const double h1 = std::max(h2, current->f - neighbor_time_point.interval.t_min);
        open_list.push(node_pool.add_node(SIPPNode(neighbor_time_point, neighbor_time_point.interval.t_min, h1, h1, 0, current)));

        if (settings.info_type != INFO_type::no_info)
        {
          generated_this_iter++;
        }
      }
    }

    // save the iteration info
    if (settings.info_type != INFO_type::no_info)
    {
      update_iter_info(*current, false);
    }
  }
  // reset the node pool
  node_pool.merge_extra();

  return TimePointPath();
}

void SIPP::initialize_iter_info()
{
  generated_this_iter = 0;
  expanded_this_iter  = 0;
  iteration_num       = -1;
}

void SIPP::update_iter_info(const sipp::SIPPNode& expanded, const bool last_iter)
{
  expanded_this_iter += 1;

  // store information about the expanded node for visualization only
  if (settings.info_type == INFO_type::visualisation)
  {
    iter_info.emplace_back(expanded.time_point, expanded.g, expanded.h, expanded.h2, expanded.h3, generated_this_iter, expanded_this_iter,
                           iteration_num);
  }

  if (last_iter)
  {
    // update the info across all sipp runs
    generated_all_iter += generated_this_iter;
    expanded_all_iter += expanded_this_iter;
  }
}

auto sipp::extract_path(SIPPNode const* const final_node) -> TimePointPath
{
  TimePointPath   path;
  SIPPNode const* curr = final_node;
  SIPPNode const* prev = nullptr;
  while (curr != nullptr)
  {
    if (prev == nullptr)  // push the whole last interval
    {
      path.push_back(curr->time_point);
    }
    else  // push only the occupied part of other intervals
    {
      path.push_back(
          TimePoint(curr->time_point.location, TimeInterval(curr->time_point.interval.t_min, prev->time_point.interval.t_min - 1)));
    }
    prev = curr;
    curr = curr->parent;
  }
  std::reverse(path.begin(), path.end());
  return path;
}
