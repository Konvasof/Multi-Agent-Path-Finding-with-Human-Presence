/**
 * @file
 * @brief Contains everything related to the visualizer.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 08-01-2025
 */

#pragma once
// #include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <iomanip>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <atomic>
#include <memory>
#include <thread>

#include "Computation.h"
#include "Instance.h"
#include "IterInfo.h"
#include "SharedData.h"

/**
 * @brief Enum class representing the type of highlight of cells to be used in the visualization.
 */
enum class Highlight
{
  NONE,
  SIPP_EXPANDED,
  LNS_DESTROYED
};

/**
 * @brief Class representing a visualization of something.
 */
class Visualization
{
public:
  int                                     time              = 0;     /**< Current time step of the visualization. */
  int                                     max_time          = 100;   /**< Maximum time step of the visualization. */
  bool                                    is_playing        = false; /**< Flag indicating whether the visualization is currently playing. */
  int                                     iteration_counter = 0;     /**< Counter for the number of iterations. */
  float                                   speed             = 2.0;   /**< Speed of the visualization. */
  std::function<void(Visualization& vis)> update_func;               /**< Function to update the visualization. */

  /**
   * @brief Constructs a Visualization object.
   *
   * @param update_func_
   */
  explicit Visualization(std::function<void(Visualization& vis)> update_func_);

  /**
   * @brief Starts the visualization.
   */
  void start();

  /**
   * @brief Resets the visualization.
   */
  void reset();

  /**
   * @brief Pauses the visualization.
   */
  void pause();

  /**
   * @brief Updates the visualization.
   */
  void update();

  /**
   * @brief Takes one step forward in the visualization.
   */
  void step_forward();

  /**
   * @brief Takes one step backward in the visualization.
   */
  void step_backward();

  /**
   * @brief Call this when new data for the visualization arrive.
   *
   * @param new_len The new length of the visualization.
   */
  void new_data(int new_len);

  /**
   * @brief Increase the iteration counter and update the visualization. Stop the visualization if the maximum time is reached.
   */
  void next_iteration();
};

/**
 * @brief Struct with info about the instance.
 * @deprecated This struct is deprecated and should be removed in the future.
 * @todo: Remove deprecated VIsualizer settings
 */
struct Settings
{
  std::string map_fname;     /**< Path to the map file. */
  std::string scene_fname;   /**< Path to the scene file. */
  int         num_of_agents; /**< Number of agents. */
};

/**
 * @brief Class,which represents the visualizer, and encapsulates all visual elements.
 * @todo Instance loading does not work.
 * @todo Resizing of the panels should also resize the sfml part.
 */
class Visualizer
{
public:
  /**
   * @brief Constructs a Visualizer object.
   *
   * @param instance_ The instance to be visualized.
   * @param computation_thread_ The computation thread.
   * @param shared_data_ The shared data between visualizer and computation thread.
   * @param seed The seed for the random number generator.
   */
  Visualizer(Instance& instance_, Computation& computation_thread_, SharedData& shared_data_, int seed);

  /**
   * @brief Destructor for the Visualizer class.
   */
  ~Visualizer();

  /**
   * @defgroup thread_management Management of threads.
   * @{
   */

  /**
   * @brief Starts the visualization thread.
   */
  void start();

  /**
   * @brief Stops the visualization thread.
   */
  void stop();

  /**
   * @brief Joins the visualization thread.
   */
  void join_thread();
  /**@}*/

  /**
   * @brief The function that runs in the visualization thread.
   */
  void run();

  /**
   * @brief Create a texture from a map.
   *
   * @param map_data The map to be used for creating the texture.
   */
  void mapToTexture(const Map& map_data);

  /**
   * @brief Update the circles shapes in the visualization with the positions of agents.
   *
   * @param positions The positions of the agents.
   */
  void update_agent_circles(const std::vector<Point2d>& positions);

  /**
   * @brief Update the goal circles shapes in the visualization with the positions of goals.
   *
   * @param positions The positions of the goals.
   */
  void update_goal_circles(const std::vector<Point2d>& positions);

  /**
   * @brief Update the visualization of a path for a given agent.
   *
   * @param agent_num The number of the agent.
   * @param path The path to be visualized.
   */
  void update_path_vis(int agent_num, const Path& path);

  /**
   * @brief Update the visualizations of all paths.
   *
   * @param time The time step to index the paths.
   */
  void update_all_paths(int time);

  /**
   * @brief Update the visualization of all paths.
   *
   * @param time The time step to index the paths.
   */
  void update_all_path_vis(int time);

  /**
   * @brief Update all highlighted cells.
   */
  void update_cell_rectangles_highlights();

  /**
   * @brief Update the sipp info for the visualization. Iterates from the current lns iteration backwards and find the closest sipp info for
   * the selected agent.
   */
  void update_sipp_info();

  /**
   * @brief Generate rectangles, that represents the cells in the map.
   */
  void create_cell_rectangles();

  void load_human_path(const std::string& filename);

  /**
   * @brief Get position of a given agent at a given time step.
   *
   * @param agent_num The number of the agent.
   * @param time The time step.
   *
   * @return The position of the agent at the given time step.
   */
  auto get_agent_position(int agent_num, int time) const -> const Point2d;

  /**
   * @brief Get location of a given agent at a given time step.
   *
   * @param agent_num The number of the agent.
   * @param time The time step.
   *
   * @return The location of the agent at the given time step.
   */
  auto get_agent_location(int agent_num, int time) const -> int;

  /**
   * @brief Get the positions of all agents at a given time step.
   *
   * @param time The time step.
   *
   * @return A vector of positions of all agents at the given time step.
   */
  auto get_agent_positions(int time) const -> std::vector<Point2d>;

  /**
   * @brief Get the locations of all agents at a given time step.
   *
   * @param time The time step.
   *
   * @return A vector of locations of all agents at the given time step.
   */
  auto get_agent_locations(int time) const -> std::vector<int>;

  /**
   * @brief Update function of the solution visualization.
   *
   * @param vis The visualization to be updated.
   */
  void solution_visualization_update(const Visualization& vis);

  /**
   * @brief Update function of the SIPP visualization.
   *
   * @param vis The visualization to be updated.
   */
  void sipp_visualization_update(const Visualization& vis);

  /**
   * @brief Update function of the LNS visualization.
   *
   * @param vis The visualization to be updated.
   */
  void lns_visualization_update(const Visualization& vis);

  /**
   * @brief Dialog window asking for the number of agents.
   */
  void number_of_agents_dialog();


private:
  /**
   * @brief Load fonts and icons.
   */
  void load_fonts();

  void set_futuristic_theme(); //NOVE

  /**
   * @brief Adjust the viewport of the SFML part of the window.
   *
   * @param window The window, where everything is rendered.
   * @param view  The view, that is used for rendering.
   * @param left_panel_size The size of the left panel.
   * @param right_panel_size The size of the right panel.
   * @param bottom_panel_size The size of the bottom panel.
   */
  void adjust_sfml_part_viewport(sf::RenderWindow& window, sf::View& view, float left_panel_size, float right_panel_size,
                                 float bottom_panel_size);

  /**
   * @brief Process events in the SFML window.
   *
   * @param window The window, where everything is rendered.
   * @param view The view, that is used for rendering.
   */
  void process_events(sf::RenderWindow& window, sf::View& view);

  /**
   * @brief Generate a random color. To make the color palette look more visually pleasing, the random color is mixed with (mix_r, mix_g,
   * mix_b).
   *
   * @param mix_r The red channel value of the mix color, recommended value is 255.
   * @param mix_g The green channel value of the mix color, recommended value is 255.
   * @param mix_b The blue channel value of the mix color, recommended value is 255.
   * @param mix_ratio The ratio at which to blend the randomly generated color with the mix color.
   *
   * @return A random color.
   */
  auto generate_random_color(int mix_r, int mix_g, int mix_b, double mix_ratio = 0.5) -> sf::Color;

  /**
   * @brief Create the left panel of the window.
   */
  void create_left_panel();

  /**
   * @brief Create the right panel of the window.
   */
  void create_right_panel();

  /**
   * @brief Create the bottom panel of the window.
   */
  void create_bottom_panel();

  /**
   * @brief Create a playback bar for the visualization.
   *
   * @param vis_name The name of the visualization.
   * @param vis The visualization to be used.
   * @param id The id used for generating unique ids of the visual elements. Will create ids in range [id, id + 6].
   */
  static void create_visualization_playback_bar(std::string vis_name, Visualization& vis, int id);

  /**
   * @brief Draw the agents.
   *
   * @param window The window, where everything is rendered.
   */
  void draw_agents(sf::RenderWindow& window);

  /**
   * @brief Draw the goals.
   *
   * @param window The window, where everything is rendered.
   */
  void draw_goals(sf::RenderWindow& window);

  /**
   * @brief Draw the path visualizations.
   *
   * @param window The window, where everything is rendered.
   */
  void draw_paths(sf::RenderWindow& window);

  /**
   * @brief Draw the cells.
   *
   * @param window The window, where everything is rendered.
   */
  void draw_cell_rectangles(sf::RenderWindow& window);

  /**
   * @brief Add a message to the log, which is displayed in the console.
   *
   * @param message The message to be added to the log.
   */
  void add_to_log(std::string message);

  std::thread  vis_thread;         /**< the underlying thread for the visualizer */
  bool         running;            /**< indicator whether the thread should be running */
  Instance&    instance;           /**< the instance to be visualized */
  Computation& computation_thread; /**< the computation thread */
  SharedData&  shared_data;        /**< the shared data between visualizer and computation thread */

  /**
   * @brief Create the docking space for the window.
   */
  void create_docking_space();

  /**
   * @brief Create the menu bar for the window.
   */
  void create_menu_bar();

  // drawing objects
  sf::Texture                                  map_texture;                /**< the map texture */
  sf::Sprite                                   map_sprite;                 /**< the sprite that represents the map */
  std::vector<sf::CircleShape>                 agents_circles;             /**< the circles that represent the agents */
  std::vector<sf::CircleShape>                 goal_circles;               /**< the circles that represent the goals */
  std::vector<sf::Color>                       agents_colors;              /** the colors of the agents */
  std::vector<sf::VertexArray>                 agents_paths_vis;           /** the path visualizations */
  std::vector<Path> const*                     agents_paths = nullptr;     /** the paths of the agents */
  std::vector<std::vector<sf::RectangleShape>> cell_rectangles;            /** the rectangles that represent the cells */
  std::vector<std::vector<Highlight>>          cell_rectangles_highlights; /** the highlights of the cells */
  SIPPInfo const*                              sipp_info;                  /** the SIPP info for the selected agent */
  std::vector<LNSIterationInfo>                lns_info;                   /** the LNS info for the selected agent */
  int                                          selected_agent = -1;        /** the selected agent */
  bool                                         is_moving      = false;     /** flag indicating whether the window is moving */
  bool                     highlight_destroyed = false; /** flag indicating whether the destroyed paths should be highlighted */
  sf::Vector2i             last_click_pos;              /** the last click position */
  sf::RectangleShape       curr_expanded_rectangle;     /** the rectangle that represents the currently expanded cell */
  std::vector<std::string> log;                         /** the log of messages that should be shown in the console */

  // Zoom settings
  float       zoom_level = 1.0;  /**< the current zoom level */
  const float zoom_step  = 1.1F; /**< the zoom step */

  // Visualizations
  Visualization solution_vis;  /**< The visualization of the solution. */
  Visualization sipp_vis;      /**< The visualization of SIPP. */
  Visualization lns_vis;       /**< The visualization of LNS. */
  int           seed;          /**< The seed for the random number generator. */
  std::mt19937  rnd_generator; /**< The random number generator. */
  Clock         clock;         /**< The clock for time measurement, used for generating timestamps for log messages. */

  Settings settings;                      /**< The settings for the visualizer. */
  bool     show_agent_num_dialog = false; /**< Flag indicating whether the dialog for the number of agents should be shown. */


  // Cesta člověka (naplníš ji v konstruktoru nebo přes SharedData)
  std::vector<Point2d> human_path_data; 
  // Grafická reprezentace dveří (např. obdélníky)
  //std::vector<sf::CircleShape> door_shapes;

  std::vector<sf::RectangleShape> door_panels;
  std::vector<sf::CircleShape>    door_knobs;

  //sf::Texture human_texture;
  //sf::Sprite  human_sprite;
  //bool        human_texture_loaded = false;

  void draw_human(sf::RenderWindow& window);
  void draw_doors(sf::RenderWindow& window);
  bool show_paths = false; 
};
