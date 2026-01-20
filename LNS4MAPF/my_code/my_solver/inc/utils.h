/**
 * @file
 * @brief Utility functions and classes for the project.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 08-01-2025
 */

#pragma once

//#include <omp.h>

#include <SFML/Graphics.hpp>
#include <omp.h>
#include <any>
#include <cassert>
#include <chrono>
#include <climits>
#include <cstdint>
#include <vector>

/**
 * @brief Assert macro that allows for a message to be printed when the assertion fails.
 *
 * @param exp The expression to assert.
 *
 * @return The result of the expression.
 * @note  Uses (void) to silence unused warnings.
 */
#define assertm(exp, msg) assert((void(msg), exp))

// time measuring
/**
 * @brief A type alias for the duration of time in seconds.
 */
using dsec = std::chrono::duration<double>;

/**
 * @brief An alias for the steady clock for time measurements.
 */
using Time = std::chrono::steady_clock;

/**
 * @brief An enum for representing directions.
 */
enum class Direction
{
  NONE  = 0,
  UP    = 1,
  RIGHT = 2,
  DOWN  = 3,
  LEFT  = 4
};

/**
 * @brief An enum for representing the type of SIPP implementation.
 */
enum class SIPP_implementation
{
  SIPP_mine,
  SIPP_mine_ap,
  SIPP_suboptimal,
  SIPP_suboptimal_ap,
  SIPP_mapf_lns,
};

/**
 * @brief An enum for representing the type of SIPP algorithm.
 */
enum class INFO_type
{
  no_info,
  visualisation,
  experiment
};

/**
 * @brief A class that wraps OpenMP locks for parallelization.
 */
struct omp_locks
{
  /**
   * @brief Constructor for the omp_locks class.
   *
   * @param num_cells The number of cells to create locks for.
   */
  explicit omp_locks(int num_cells);

  /**
   * @brief Destructor for the omp_locks class.
   */
  ~omp_locks();

  std::vector<omp_lock_t> locks;  ///< Vector of OpenMP locks
};

/**
 * @brief A class for time measurement.
 */
class Clock
{
public:
  /**
   * @brief Start the clock.
   */
  void start();

  /**
   * @brief Get current time without stopping the clock.
   *
   * @return A pair of doubles representing the current wall time and CPU time.
   */
  [[nodiscard]] auto get_current_time() const -> std::pair<double, double>;

  /**
   * @brief Stop the clock and return the elapsed time.
   *
   * @return A pair of doubles representing the elapsed wall time and CPU time.
   */
  auto end() -> std::pair<double, double>;

private:
  bool             initialized = false;  ///< Flag to check if the clock is initialized
  Time::time_point wall_time_start;      ///< Start time for wall clock
  timespec         cpu_time_start{};     ///< Start time for CPU clock
};


/**
 * @brief A class representing a node in a search algorithm.
 */
class Node
{
public:
  double g, h, f;  ///< Cost values for the node

  /**
   * @brief Constructor for the Node class.
   *
   * @param g_ The g value (cost from start to this node).
   * @param h_ The h value (estimated cost from this node to the goal).
   */
  Node(double g_, double h_);

  /**
   * @brief Virtual destructor for the Node class. It makes Node an abstract class.
   */
  virtual ~Node() = default;

  /**
   * @brief Compare two nodes based on their f values.
   *
   * @param other The other node to compare.
   *
   * @return True if this node's f value is >= than the other node's f value, false otherwise.
   */
  virtual auto operator>(const Node& other) -> bool;

  /**
   * @brief Compare two nodes based on their f values.
   *
   * @param other The other node to compare.
   *
   * @return True if this node's f value is < than the other node's f value, false otherwise.
   */
  virtual auto operator<(const Node& other) -> bool;

  /**
   * @brief Check if two nodes are equal.
   *
   * @param other The other node to compare.
   *
   * @return True if the two nodes are equal, false otherwise.
   */
  virtual auto operator==(const Node& other) -> bool;
};

/**
 * @brief A class representing a 2D point.
 */
class Point2d
{
public:
  int x, y;  ///< Coordinates of the point
             ///
  /**
   * @brief Constructor for the Point2d class.
   *
   * @param x_ The x-coordinate of the point.
   * @param y_ The y-coordinate of the point.
   */
  Point2d(int x_, int y_);

  /**
   * @brief Destructor for the Point2d class.
   */
  ~Point2d() = default;

  /**
   * @brief Add two points.
   *
   * @param other The other point to add.
   *
   * @return A new point that is the sum of the two points.
   */
  auto operator+(const Point2d& other) const -> Point2d;

  /**
   * @brief Subtract two points.
   *
   * @param other The other point to subtract.
   *
   * @return A new point that is the difference of the two points.
   */
  auto operator-(const Point2d& other) const -> Point2d;


  /**
   * @brief Check if two points are equal.
   *
   * @param other The other point to compare.
   *
   * @return True if the two points are equal, false otherwise.
   */
  auto operator==(const Point2d& other) const -> bool;


  /**
   * @brief Add a point to this point.
   *
   * @param other The other point to add.
   */
  void add(const Point2d& other);

  /**
   * @brief Operator for printing the point.
   *
   * @param os The output stream.
   * @param p The point to print.
   *
   * @return The output stream.
   */
  friend auto operator<<(std::ostream& os, Point2d const& p) -> std::ostream&;

  /**
   * @brief Hash function for the Point2d class.
   */
  struct hash_point
  {
    auto operator()(const Point2d& p) const -> size_t;
  };


  /**
   * @brief Convert the point to a string representation.
   *
   * @return A string representation of the point.
   */
  explicit operator std::string() const;
};

/**
 * @brief Find the direction between two points.
 */
auto find_direction(const Point2d& start, const Point2d& end) -> enum Direction;

/**
 * @brief Find the direction between two locations.
 */
auto find_direction(int start, int end) -> enum Direction;

/**
 * @brief A class that represents a time interval.
 */
class TimeInterval
{
public:
  int t_min, t_max;  ///< Minimum and maximum time values

  /**
   * @brief Constructor for the TimeInterval class.
   *
   * @param t_min_ The minimum time value.
   * @param t_max_ The maximum time value.
   */
  TimeInterval(int t_min_, int t_max_);

  /**
   * @brief The equality check operator for the TimeInterval class.
   *
   * @param other The other time interval to compare.
   *
   * @return True if the two time intervals are equal, false otherwise.
   */
  auto operator==(const TimeInterval& other) const -> bool;

  /**
   * @brief The addition operator for the TimeInterval class.
   *
   * @param x The value to add to the time interval.
   *
   * @return A new time interval that is shifted by the value x.
   */
  auto operator+(int x) const -> TimeInterval;

  /**
   * @brief Operator to convert the time interval to a string representation.
   *
   * @return A string representation of the time interval.
   */
  explicit operator std::string() const;

  /**
   * @brief Operator for printing the time interval.
   */
  friend auto operator<<(std::ostream& os, TimeInterval const& i) -> std::ostream&;
};

/**
 * @brief Check if two time intervals overlap.
 *
 * @param i1 The first time interval.
 * @param i2 The second time interval.
 *
 * @return True if the two time intervals overlap, false otherwise.
 */
auto overlap(const TimeInterval& i1, const TimeInterval& i2) -> bool;

/**
 * @brief A class that represents a location along with its time interval.
 */
class TimePoint
{
public:
  int          location;  ///< Location of the time point
  TimeInterval interval;  ///< Time interval of the time point

  /**
   * @brief Constructor for the TimePoint class.
   *
   * @param location The location of the time point.
   * @param interval The time interval of the time point.
   */
  TimePoint(int location, const TimeInterval& interval);

  /**
   * @brief The equality check operator for the TimePoint class.
   *
   * @param other The other time point to compare.
   *
   * @return True if the two time points are equal, false otherwise.
   */
  auto operator==(const TimePoint& other) const -> bool;
};

/**
 * @brief An alias for a vector of locations.
 */
using Path = std::vector<int>;

/**
 * @brief An alias for a vector of 2d points.
 */
using PointPath = std::vector<Point2d>;

/**
 * @brief An alias for a vector of TimePoints.
 */
using TimePointPath = std::vector<TimePoint>;

/**
 * @brief Convert a TimePointPath to a Path.
 *
 * @param tp_path The TimePointPath to convert.
 *
 * @return The converted Path.
 */
auto timepointpath_to_path(const TimePointPath& tp_path) -> Path;

/**
 * @brief Convert a Path to a TimePointPath.
 *
 * @param path The Path to convert.
 *
 * @return The converted TimePointPath.
 */
auto path_to_timepointpath(const Path& path) -> TimePointPath;

/**
 * @brief Check, whether the intervals of a TimePointPath overlap.
 *
 * @param tp_path The TimePointPath to check.
 *
 * @return True if the intervals have no overlap, false otherwise.
 */
auto check_timepointpath_interval_no_overlap(const TimePointPath& tp_path) -> bool;

/**
 * @brief Fix the intervals of a TimePointPath, so that they do not overlap.
 *
 * @param tp_path The TimePointPath to fix.
 */
void fix_timepointpath_interval_overlap(TimePointPath& tp_path);

/**
 * @brief Check if a TimePointPath is valid.
 *
 * @param tp_path The TimePointPath to check.
 *
 * @return True if the TimePointPath is valid, false otherwise.
 */
auto is_valid_timepointpath(const TimePointPath& tp_path) -> bool;

/**
 * @brief Get the path to the project directory, assuming the project is run from the build directory. This function is useful for loading
 * files from the project directory.
 *
 * @return The path to the project directory.
 */
auto get_base_path() -> std::string;

/**
 * @brief A overflow-safe function to increase an integer value.
 *
 * @param num The integer value to increase.
 *
 * @return The increased value, or INT_MAX if the value is already INT_MAX.
 */
inline auto safe_increase(int num) -> int
{

  if (num == INT_MAX)
  {
    return INT_MAX;
  }
  return num + 1;
}


/**
 * @brief Convert an any type to a string representation.
 *
 * @param value The any type value to convert.
 *
 * @return A string representation of the value.
 */
auto any_to_str(const std::any& value) -> std::string;

/**
 * @brief Convert a double to a string representation.
 *
 * @param num The double value to convert.
 * @param separating_char The character to use for separating thousands.
 * @param precision The number of decimal places to include in the string.
 *
 * @return A string representation of the double value.
 */
auto double_to_str(double num, char separating_char = ',', int precision = 2) -> std::string;

/**
 * @brief Find the last number in a string.
 *
 * @param str The string to search.
 *
 * @return The last number found in the string, or -1 if no number is found.
 */
auto find_last_number(const std::string& str) -> int;

/**
 * @brief Find the location, that is visited by a given trajectory at a given time.
 *
 * @param path The trajectory to check.
 * @param time The time to check.
 *
 * @return The location that is visited by the trajectory at the given time.
 */
auto location_at_time(TimePointPath& path, int time) -> int;

/**
 * @brief Open a file dialog to select a file.
 *
 * @param type The type of file to select.
 * @param file_extensions The file extensions to filter the files (e.g., "txt, csv").
 *
 * @return The selected file path as a string.
 */
auto open_file_dialog(const std::string& type, const std::string& file_extensions) -> std::string;
