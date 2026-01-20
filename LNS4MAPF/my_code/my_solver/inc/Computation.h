/**
 * @file
 * @brief Defines the Computation class, which handles all computation, invokes the solver, and manages the thread.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 08-01-2025
 */

#pragma once
#include <atomic>
#include <memory>
#include <random>
#include <thread>

#include "Instance.h"
#include "LNS.h"
#include "SharedData.h"

/**
 * @brief Handles all computation, invokes the solver, and manages the thread.
 *
 */
class Computation
{
public:
  /**
   * @brief Constructs a Computation object.
   *
   * @param instance_ The instance to be solved.
   * @param shared_data_ The shared data object for communication between threads.
   * @param lns_settings_ The settings for the LNS solver.
   * @param seed The seed for the random number generator.
   */
  Computation(const Instance& instance_, SharedData* shared_data_, LNS_settings lns_settings_, int seed);

  void set_safety_params(bool safety_aware, int human_start, int door_loc);


  /**
   * @brief Destructor for the Computation class.
   */
  ~Computation();

  // manage the underlying thread
  /**
   * @brief Starts the computation thread.
   */
  void start();

  /**
   * @brief Stops the computation thread.
   */
  void stop();

  /**
   * @brief Joins the computation thread.
   */
  void join_thread();

  /**
   * @brief Retrieve the solution.
   * @warning This function should only be called after the solver has finished.
   * @throws std::runtime_error if the solver is still running.
   *
   * @return A constant reference to the solution object.
   */
  auto get_solution() -> const Solution&
  {
    if (running.load(std::memory_order_acquire))
    {
      throw std::runtime_error("Trying to access the solution before the solver finished.");
    }
    return solver->solution;
  }

  /**
   * @brief The main function that runs the computation, which is called in the thread.
   */
  void run();

  std::atomic<bool> running; /**< Indicates whether the computation is running. */

private:
  std::unique_ptr<LNS> solver;      /**< Pointer to the LNS solver. */
  std::thread          comp_thread; /**< The computation thread. */

  const Instance& instance;      /**< The instance to be solved. */
  SharedData*     shared_data;   /**< Pointer to the shared data object. */
  int             seed;          /**< The seed for the random number generator. */
  std::mt19937    rnd_generator; /**< Random number generator. */
  LNS_settings    lns_settings;  /**< The settings for the LNS solver. */
};
