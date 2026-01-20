/**
 * @file
 * @brief Defines the Experiment class, which is used for experiments.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 06-04-2025
 */

#pragma once
#include <sqlite3.h>

#include <string>
#include <vector>

#include "LNS.h"
#include "experiment_utils.h"

/**
 * @brief The name of the database file.
 */
#define DATABASE_NAME "experiment_database.db"

/**
 * @brief A class that represents a single experiment.
 *
 * It takes care of instance loading, skipping already performed experiments, result saving, loading seeds from the database and more.
 */
class Experiment
{
public:
  /**
   * @brief Constructs an Experiment object.
   *
   * @param experiment_name_ The name of the experiment.
   * @param experiment_function The function to be executed for each experiment.
   * @param maps_ A vector of map names.
   * @param agent_nums_ A vector of vectors of agent numbers.
   * @param time_limits_ A vector of time limits for each map and each number of agents.
   * @param algorithms_ A vector of algorithms to be used in the experiment.
   * @param runs_per_map_ The number of runs per map.
   * @param show_progress_ Whether to show progress bars.
   * @param load_init_sol Whether to load initial solutions from files.
   */
  Experiment(std::string experiment_name_, std::function<void(LNS& lns)> experiment_function, std::vector<std::string> maps_,
             std::vector<std::vector<int>> agent_nums_, std::vector<double> time_limits_, std::vector<Algorithm>& algorithms_,
             int runs_per_map_, bool show_progress_ = false, bool load_init_sol = false);

  /**
   * @brief A destructor for the Experiment class. Also closes the database connection if it is open.
   */
  ~Experiment()
  {
    if (db != nullptr)
    {
      sqlite3_close(db);
    }
  }

  /**
   * @brief Runs the experiment.
   */
  void run();

private:
  /**
   * @brief Initializes the progress bars.
   *
   * @param agent_nums_init The number of experiments that will be performed for the first map. The progress bar will show progress as x/Y,
   * where Y is this number.
   */
  void create_progress_bars(int agent_nums_init);

  /**
   * @brief Opens connection to the experiment database.
   */
  void open_database();

  /**
   * @brief Retrieves the seed from the experiment database.
   *
   * @param agent_num Number of agents used in the experiment.
   * @param map_name Name of the map used in the experiment.
   * @param scene_name_stripped Name of the scene used in the experiment.
   * @param run The run number of the experiment.
   *
   * @return The seed loaded from the database.
   * @todo The name of the experiment, whose seed is loaded from the database is different for each experiment, so it is hardcoded in the
   * function code. There must be a more elegant way to achieve this
   */
  auto get_seed_from_db(int agent_num, const std::string& map_name, const std::string& scene_name_stripped, int run) -> int;

  /**
   * @brief Finds path to the initial solution file. Assumes, the initial solution files are stored in
   * /experiments/$experiment_name$_initsols/
   *
   * @param agent_num Number of agents used in the experiment.
   * @param map_name Name of the map used in the experiment.
   * @param scene_name_stripped Name of the scene used in the experiment, without the map name and the extension.
   * @param run The run number of the experiment.
   *
   * @return The path to the initial solution file.
   */
  auto get_initsol_path(int agent_num, const std::string& map_name, const std::string& scene_name_stripped, int run) -> std::string;

  std::string                         experiment_name;     /**< Name of the experiment */
  std::function<void(LNS& lns)>       experiment_function; /**< Function to be executed for each experiment */
  const std::vector<std::string>      maps;                /**< Vector of map names */
  const std::vector<std::vector<int>> agent_nums;          /**< Agent numbers for each map */
  const std::vector<double>           time_limits;         /**< Time limits for each map */
  std::vector<Algorithm>&             algorithms;          /**< Algorithms to be used in the experiment */
  int                                 runs_per_map;        /**< Number of runs per map */
  bool                                show_progress;       /**< Whether to show progress bars */

  std::vector<bool> skips; /**< Vector of booleans indicating whether to skip the experiment for each map and agent number */
  int               skipped_maps            = 0;     /**< Number of skipped maps */
  int               skipped_agent_nums      = 0;     /**< Number of skipped agent numbers */
  int               skipped_runs            = 0;     /**< Number of skipped runs */
  int               skipped_implementations = 0;     /**< Number of skipped implementations */
  int               sum_agent_nums          = 0;     /**< Sum of agent numbers that is continuously updated, used for skipping */
  bool              load_init_sol           = false; /**< Whether to load initial solutions from files */


  std::mt19937                       rnd_gen; /**< Random number generator */
  std::uniform_int_distribution<int> dist;    /**< Uniform distribution for random numbers */

  // visualization
  std::vector<std::unique_ptr<indicators::ProgressBar>> progress_bars; /**< Vector of progress bars */
  std::optional<indicators::MultiProgress<indicators::ProgressBar, 3>>
      multi_progress_bar; /**< Multi progress bar which wrappes around all the individual progress bars */

  sqlite3* db = nullptr; /**< SQLite database connection */
};
