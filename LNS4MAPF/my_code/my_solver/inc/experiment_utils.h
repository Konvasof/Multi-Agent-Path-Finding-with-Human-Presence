/**
 * @file
 * @brief Functions for experiments.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 06-03-2025
 */

#pragma once

#include <any>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <random>
#include <variant>

#include "Instance.h"
#include "LNS.h"
#include "indicators/indicators.hpp"
#include "utils.h"

/**
 * @brief Width of the progress bar.
 */
#define PROGRESS_BAR_WIDTH 100

/**
 * @brief Algorithm type is one of the SIPP implementations.
 */
using AlgorithmType = std::variant<SIPP_implementation>;

/**
 * @brief Algorithm class represents an algorithm type and its parameters.
 */
class Algorithm
{
public:
  AlgorithmType                                 type;         /**< Type of the algorithm. */
  LNS_settings                                  lns_settings; /**< LNS settings of the algorithm */
  std::vector<std::pair<std::string, std::any>> parameters;   /** Parameters of the algorithm. */

  /**
   * @brief Constructor of the Algorithm class. It is a variadic constructor, that acceppts any number of parameters.
   *
   * @tparam Args The list of algorithm parameters.
   * @param type_ The type of the algorithm.
   * @param lns_settings_ The LNS settings of the algorithm.
   * @param args The list of algorithm parameters.
   */
  template <typename... Args>
  Algorithm(AlgorithmType type_, LNS_settings lns_settings_, Args&&... args) : type(type_), lns_settings(lns_settings_)
  {
    (parameters.emplace_back(std::forward<Args>(args)), ...);
  }

  /**
   * @brief Add a parameter to the algorithm.
   *
   * @tparam T The type of the parameter.
   * @param param The parameter to add.
   */
  template <typename T>
  void add_parameter(const T& param)
  {
    parameters.push_back(param);
  }

  /**
   * @brief Getter for the algorithm name.
   *
   * @return  The name of the algorithm.
   */
  [[nodiscard]] auto get_name() const -> std::string;

  /**
   * @brief Get a string representation of the parameters of the algorithm.
   *
   * @return  The parameters of the algorithm as a string.
   */
  [[nodiscard]] auto get_parameters_str() const -> std::vector<std::pair<std::string, std::string>>;
};

/**
 * @brief Get the path to the mapf_benchmark directory.
 *
 * @return  The path to the mapf_benchmark directory.
 */
auto get_mapf_benchmark_path() -> std::string;

/**
 * @brief Get names of the scenes for the given map and number of runs.
 *
 * @param map_name The name of the map.
 * @param runs_per_map The number of runs for the map.
 *
 * @return The names of the scenes for the given map and number of runs.
 */
auto get_scene_names(const std::string& map_name, int runs_per_map) -> std::vector<std::string>;

/**
 * @brief Create a progress bar.
 *
 * @param name The name of the progress bar.
 * @param max_iter The maximum number of iterations.
 *
 * @return A unique pointer to the progress bar.
 */
auto create_progress_bar(std::string name, int max_iter) -> std::unique_ptr<indicators::ProgressBar>;

/**
 * @brief Load instances from the given scenes.
 *
 * @param scenes The names of the scenes.
 * @param map_name The name of the map.
 * @param agent_num The number of agents.
 *
 * @return The loaded instances, and the preprocessing times as pairs of wall time and cpu time.
 */
auto load_instances(const std::vector<std::string>& scenes, const std::string& map_name, int agent_num)
    -> std::pair<std::vector<Instance>, std::vector<std::pair<double, double>>>;

/**
 * @brief Save the experiment results to a JSON file.
 *
 * @param experiment_res The JSON object to save the results to.
 * @param agent_num The number of agents.
 * @param map_name The name of the map.
 * @param scene_name The name of the scene.
 * @param experiment_name The name of the experiment.
 * @param algorithm_name The name of the algorithm.
 */
void save_experiment(nlohmann::json& experiment_res, int agent_num, const std::string& map_name, const std::string& scene_name,
                     const std::string& experiment_name, const std::string& algorithm_name);

/**
 * @brief Determine, which experiments were already performed and can be skipped.
 *
 * @param experiment_name The name of the experiment.
 * @param maps The names of the maps.
 * @param agent_nums The number of agents for each map.
 * @param runs_per_map The number of runs for each map.
 * @param algorithms The algorithms to be used for the experiment.
 *
 * @return A vector of booleans, where true means that the experiment can be skipped.
 */
auto what_can_be_skipped(const std::string& experiment_name, const std::vector<std::string>& maps,
                         const std::vector<std::vector<int>>& agent_nums, int runs_per_map, const std::vector<Algorithm>& algorithms)
    -> std::vector<bool>;
