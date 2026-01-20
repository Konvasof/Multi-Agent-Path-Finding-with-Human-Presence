/**
 * @file
 * @brief Contains classes for storing information about the iterations of the algorithm, which are used for logging and debugging.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 22-02-2025
 */
#pragma once
#include <utility>

#include "Solver.h"
#include "utils.h"

/**
 * @brief Stores information about iterations of the SIPP algorithm.
 */
class SIPPIterationInfo
{
public:
  const TimePoint cur_expanded;  /**< TimePoint expanded at this iteration */
  const double    g;             /**< Travel cost (g value) of the represented node */
  const double    h;             /**< Primary heuristic cost (h value) of the currently represented node */
  const double    h2;            /**< Secondary heuristic cost (h2 value) of the currently represented node */
  const double    h3;            /**< Terciary  heuristic cost (h3 value) of the currently represented node */
  const int       generated;     /**< Number of generated nodes */
  const int       expanded;      /**< Number of expanded nodes */
  const int       iteration_num; /**< Iteration number of the represented node */

  /**
   * @brief Constructor for SIPPIterationInfo.
   *
   * @param cur_expanded_ Current expanded time point
   * @param g_ Travel cost (g value) of the currently expanded node
   * @param h_ Primary heuristic cost (h value) of the currently expanded node
   * @param h2_ Secondary heuristic cost (h2 value) of the currently expanded node
   * @param h3_ Terciary heuristic cost (h3 value) of the currently expanded node
   * @param generated_ Number of generated nodes
   * @param expanded_ Number of expanded nodes
   * @param iteration_num_ Current iteration number
   */
  SIPPIterationInfo(const TimePoint& cur_expanded_, double g_, double h_, double h2_, double h3_, int generated_, int expanded_,
                    int iteration_num_);
};

/**
 * @brief Alias for a vector of SIPPIterationInfo objects.
 */
using SIPPInfo = std::vector<SIPPIterationInfo>;

/**
 * @brief Stores information about the iterations of the LNS algorithm.
 */
class LNSIterationInfo
{
public:
  int                   iteration_num;    /**< Iteration number */
  bool                  accepted;         /**< Whether the solution was accepted */
  int                   improvement;      /**< Improvement of the solution */
  std::vector<SIPPInfo> sipp_info;        /**< Information about all underlying SIPP searches */
  Solution              sol;              /**< Solution object representing the current solution */
  std::vector<int>      planning_order;   /**< The priorities used in this LNS iteration */
  std::string           destroy_strategy; /**< The destroy strategy used in this LNS iteration */

  /**
   * @brief Constructor for LNSIterationInfo.
   *
   * @param iteration_num_ Iteration number
   * @param accepted_ Whether the solution was accepted
   * @param improvement_ Improvement of the solution
   * @param sipp_info_ Information about all underlying SIPP searches
   * @param sol_ Solution object representing the current solution
   * @param destroy_strategy_ The destroy strategy used in this LNS iteration
   */
  LNSIterationInfo(int iteration_num_, bool accepted_, int improvement_, std::vector<SIPPInfo>& sipp_info_, Solution sol_,
                   std::string destroy_strategy_)
      : iteration_num(iteration_num_),
        accepted(accepted_),
        improvement(improvement_),
        sipp_info(std::move(sipp_info_)),
        sol(std::move(sol_)),
        destroy_strategy(std::move(destroy_strategy_))
  {
    sol.convert_paths();
  }
};

/**
 * @brief Alias for a vector of LNSIterationInfo objects.
 */
using LNSInfo = std::vector<LNSIterationInfo>;
