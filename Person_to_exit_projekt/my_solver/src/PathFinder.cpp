#include "PathFinder.h"
#include <queue>
#include <map>
#include <cmath>
#include <algorithm>

using Point = std::pair<int, int>;

// Struktura pro Priority Queue
struct Node {
    Point pos;
    int f_score;
    
    bool operator>(const Node& other) const {
        return f_score > other.f_score;
    }
};

int AStarPathFinder::heuristic(Point a, Point b) {
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

std::optional<std::vector<Point>> AStarPathFinder::find_path(
    Point start, Point goal, const GridMap& grid_map, const std::set<Point>& dynamic_obstacles) 
{
    // Rychlá kontrola start/cíl
    if (!grid_map.is_walkable(start.first, start.second) || dynamic_obstacles.count(start)) return std::nullopt;
    if (!grid_map.is_walkable(goal.first, goal.second) || dynamic_obstacles.count(goal)) return std::nullopt;

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open_set;
    open_set.push({start, heuristic(start, goal)});

    std::map<Point, Point> came_from;
    std::map<Point, int> g_score;
    g_score[start] = 0;

    // Pro optimalizaci, abychom nevkládali do mapy zbytečně
    auto get_g = [&](Point p) {
        if (g_score.find(p) == g_score.end()) return 1000000;
        return g_score[p];
    };

    while (!open_set.empty()) {
        Point current = open_set.top().pos;
        int current_f = open_set.top().f_score;
        open_set.pop();

        // Pokud jsme v cíli
        if (current == goal) {
            std::vector<Point> total_path;
            while (current != start) {
                total_path.push_back(current);
                current = came_from[current];
            }
            total_path.push_back(start);
            std::reverse(total_path.begin(), total_path.end());
            return total_path;
        }

        // 4 Směry
        int dx[] = {0, 0, 1, -1};
        int dy[] = {1, -1, 0, 0};

        for (int i = 0; i < 4; ++i) {
            Point neighbor = {current.first + dx[i], current.second + dy[i]};

            // 1. Je ve zdi?
            if (!grid_map.is_walkable(neighbor.first, neighbor.second)) continue;
            // 2. Je tam agent?
            if (dynamic_obstacles.count(neighbor)) continue;

            int tentative_g = get_g(current) + 1;

            if (tentative_g < get_g(neighbor)) {
                came_from[neighbor] = current;
                g_score[neighbor] = tentative_g;
                int f = tentative_g + heuristic(neighbor, goal);
                open_set.push({neighbor, f});
            }
        }
    }

    return std::nullopt;
}
