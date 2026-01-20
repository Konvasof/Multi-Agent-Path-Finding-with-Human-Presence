/**
 * @file
 * @brief Contains the Map class, which represents a 2D gridmap for pathfinding.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 04-04-2025
 */
#pragma once
#include "utils.h"

/**
 * @brief Represents a 2D gridmap for pathfinding.
 */
class Map
{
public:
  int                  width  = 0;                     ///< Width of the map
  int                  height = 0;                     ///< Height of the map
  bool                 loaded = false;                 ///< Indicates if the map is loaded
  std::string          type;                           ///< Type of the map
  std::vector<int>     free_location_to_location_vec;  ///< Mapping from free locations to actual locations
  std::vector<int>     location_to_free_location_vec;  ///< Mapping from actual locations to free locations
  std::vector<uint8_t> data;                           ///< Map data (0 for free, 1 for occupied)

  /**
   * @brief Constructs a Map object.
   */
  Map() = default;

  /**
   * @brief Constructs a Map object with the specified width and height.
   */
  void print() const;

  /**
   * @brief Finds the neighbors of a given point in the map.
   *
   * @param p The point for which to find neighbors.
   *
   * @return A vector of neighboring points.
   */
  [[nodiscard]] auto find_neighbors(const Point2d& p) const -> std::vector<Point2d>;

  /**
   * @brief   Finds the neighbors of a given location in the map.
   *
   * @param loc The location for which to find neighbors.
   *
   * @return A vector of neighboring locations.
   */
  [[nodiscard]] auto find_neighbors(int loc) const -> std::vector<int>;

  /**
   * @brief   Loads a map from a file.
   *
   * @param map_fname The name of the file containing the map data.
   */
  void load(const std::string& map_fname);

  /**
   * @brief Checks, if the given point is within the bounds of the map.
   *
   * @param p The point to check.
   *
   * @return True if the point is within the map, false otherwise.
   */
  [[nodiscard]] inline auto is_in(const Point2d& p) const -> bool
  {
    assertm(loaded, "Map not loaded.");
    return !(p.x < 0 || p.x >= width || p.y < 0 || p.y >= height);
  }

  /**
   * @brief Checks, if the given location is within the bounds of the map.
   *
   * @param loc The location to check.
   *
   * @return True if the location is within the map, false otherwise.
   */
  [[nodiscard]] inline auto is_in(int loc) const -> bool
  {
    assertm(loaded, "Map not loaded.");
    return loc >= 0 && loc < width * height;
  }

  /**
   * @brief Indexes the map at the given point.
   *
   * @param point The point to index the map.
   *
   * @return The value at the given point in the map (0 for free, 1 for occupied).
   */
  [[nodiscard]] inline auto index(const Point2d& point) const -> uint8_t
  {
    assertm(loaded, "Map not loaded.");
    assertm(is_in(point), "Trying to index a point that is not in the map.");
    return data[point.y * width + point.x];
  }

  /**
   * @brief Indexes the map at the given location.
   *
   * @param loc The location to index the map.
   *
   * @return The value at the given location in the map (0 for free, 1 for occupied).
   */
  [[nodiscard]] inline auto index(int loc) const -> uint8_t
  {
    assertm(loaded, "Map not loaded.");
    assertm(is_in(loc), "Trying to index a point that is not in the map.");
    return data[loc];
  }

  /**
   * @brief Indexes the map at the given free location (free locations means locations, that are not static obstacles).
   *
   * @param free_loc The free location to index the map.
   *
   * @return The value at the given free location in the map (0 for free, 1 for occupied).
   */
  [[nodiscard]] inline auto index_free(int free_loc) const -> uint8_t
  {
    return index(free_location_to_location(free_loc));
  }

  /**
   * @brief Converts a free location index to a location index.
   *
   * @param free_loc The free location index to convert.
   *
   * @return The corresponding location index.
   */
  [[nodiscard]] inline auto free_location_to_location(int free_loc) const -> int
  {
    assertm(loaded, "Map not loaded.");
    assertm(0 <= free_loc && free_loc < static_cast<int>(free_location_to_location_vec.size()), "Invalid free location index.");
    return free_location_to_location_vec[free_loc];
  }

  /**
   * @brief Converts a location index to a free location index.
   *
   * @param loc The location index to convert.
   *
   * @return The corresponding free location index.
   */
  [[nodiscard]] inline auto location_to_free_location(int loc) const -> int
  {
    assertm(loaded, "Map not loaded.");
    assertm(index(loc) == 0, "Not a free location");
    return location_to_free_location_vec[loc];
  }

  /**
   * @brief Converts a point to a location index.
   *
   * @param p The point to convert.
   *
   * @return The corresponding location index.
   */
  [[nodiscard]] inline auto position_to_index(const Point2d& p) const -> int
  {
    assertm(loaded, "Map not loaded.");
    return p.y * width + p.x;
  }

  /**
   * @brief Getter fur the number of free cells in the map.
   *
   * @return The number of free cells in the map.
   */
  [[nodiscard]] inline auto get_num_free_cells() const -> int
  {
    assertm(loaded, "Map not loaded.");
    return static_cast<int>(free_location_to_location_vec.size());
  }

  /**
   * @brief Getter for the number of cells in the map.
   *
   * @return The number of cells in the map.
   */
  [[nodiscard]] inline auto get_num_cells() const -> int
  {
    assertm(loaded, "Map not loaded.");
    assertm(width * height == static_cast<int>(data.size()), "Width and height inconsistent with the map data.");
    return static_cast<int>(data.size());
  }
};
