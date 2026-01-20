/**
 * @file
 * @brief Data shared between computation and visualization thread.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 20-02-2025
 */
#pragma once
#include <atomic>
#include <mutex>

#include "IterInfo.h"
#include "utils.h"

/**
 * @brief A class to hold shared data between computation and visualization threads.
 */
class SharedData
{
public:
  std::atomic<bool> is_new_info; /**< Flag indicating if new information is available. */
  std::atomic<bool> is_end;      /**< Flag indicating if the solver should end. */

  /**
   * @brief Constructor for SharedData.
   */
  SharedData();

  /**
   * @brief Function to add information about a lns iteration.
   *
   * @param new_info The new information to be added.
   */
  void update_lns_info(LNSIterationInfo new_info);

  /**
   * @brief Function to consume the information about lns iterations.
   *
   * @return A vector of LNSIterationInfo objects.
   */
  auto consume_lns_info() -> std::vector<LNSIterationInfo>;

private:
  std::mutex                    mtx;      /**< Mutex for thread safety. */
  std::vector<LNSIterationInfo> lns_info; /**< Vector to hold LNS iteration information. */
};
