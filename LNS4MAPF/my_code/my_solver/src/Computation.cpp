/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "Computation.h"

#include <chrono>
#include <iostream>

#include "LNS.h"

Computation::Computation(const Instance& instance_, SharedData* shared_data_, LNS_settings lns_settings_, int seed_ = -1)
  : instance(instance_), running(false), shared_data(shared_data_), seed(seed_), lns_settings(lns_settings_){
  // initialized the random generator
  if (seed < 0)
  {
    // random device to generate a random seed
    std::random_device rd;
    rnd_generator = std::mt19937(rd());
  }
  else
  {
    rnd_generator = std::mt19937(seed);
  }

  // initialize the solver
  solver = std::make_unique<LNS>(instance, rnd_generator, shared_data, lns_settings);
}

Computation::~Computation()
{
  stop();
  join_thread();
}

void Computation::start()
{
  // start the underlying thread
  comp_thread = std::thread(&Computation::run, this);
  running.store(true, std::memory_order_release);
}

void Computation::stop()
{
  // set the stop signal
  running.store(false, std::memory_order_release);
}

void Computation::join_thread()
{
  if (comp_thread.joinable())
  {
    comp_thread.join();
  }
}

void Computation::run()
{
  solver->solve();
  if (solver->solution.feasible)
  {
    std::cout << "Final solution has sum of costs: " << solver->solution.sum_of_costs << std::endl;
  }
  else
  {
    std::cout << "Unable to find feasible solution." << std::endl;
  }
  running.store(false, std::memory_order_release);
}

void Computation::set_safety_params(bool safety_aware, int human_start, int door_loc)
{
  if (solver) {
    solver->safety_aware_mode = safety_aware;
    solver->human_start_location = human_start;
    solver->safety_exit_location = door_loc;
    
    std::cout << "Safety params set. Mode: " << safety_aware 
              << ", Human start loc: " << human_start 
              << ", Door loc: " << door_loc << std::endl;
  }
}
