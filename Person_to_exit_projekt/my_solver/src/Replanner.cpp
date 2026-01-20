#include "Replanner.h"
#include <queue>
#include <tuple>
#include <algorithm>
#include <iostream>

using namespace std;

// Stav pro A*: (x, y, čas) + f_score
struct Node {
    int x, y, t;
    int f_score;
    // Chceme nejmenší f_score, proto >
    bool operator>(const Node& other) const { return f_score > other.f_score; }
};

optional<vector<pair<int, int>>> Replanner::find_path(
    int agent_id, pair<int, int> start, pair<int, int> goal,
    const GridMap& grid,
    const map<int, vector<pair<int, int>>>& current_paths,
    const vector<pair<int, int>>& human_path,
    int max_time
) {
    priority_queue<Node, vector<Node>, greater<Node>> open_set;
    open_set.push({start.first, start.second, 0, 0});

    // Ukládáme si, odkud jsme přišli: mapa (x,y,t) -> (x,y,t-1)
    map<tuple<int,int,int>, tuple<int,int,int>> came_from;
    map<tuple<int,int,int>, int> g_score;
    
    g_score[{start.first, start.second, 0}] = 0;

    while (!open_set.empty()) {
        // OPRAVA: Musíme načíst všechny 4 hodnoty (přidáno 'ignore_f')
        auto [cx, cy, ct, ignore_f] = open_set.top();
        open_set.pop();

        // 1. Jsme v cíli?
        if (cx == goal.first && cy == goal.second) {
            // Rekonstrukce cesty zpět
            vector<pair<int, int>> path;
            tuple<int,int,int> curr = {cx, cy, ct};
            while (came_from.find(curr) != came_from.end()) {
                path.push_back({get<0>(curr), get<1>(curr)});
                curr = came_from[curr];
            }
            path.push_back(start);
            reverse(path.begin(), path.end());
            return path;
        }

        if (ct >= max_time) continue;

        // Možné pohyby: Zůstat (0,0), Nahoru, Dolů, Vlevo, Vpravo
        int dx[] = {0, 0, 0, 1, -1};
        int dy[] = {0, 1, -1, 0, 0};

        for (int i = 0; i < 5; ++i) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];
            int nt = ct + 1;

            // --- KONTROLY (Tady koukáme do souborů/mapy) ---

            // A) Je to ve zdi?
            if (!grid.is_walkable(nx, ny)) continue;

            // B) Je tam ČLOVĚK? (Safety)
            // Pokud má člověk v čase nt být na (nx, ny), nesmím tam jít.
            bool human_collision = false;
            if (nt < human_path.size()) {
                if (human_path[nt] == make_pair(nx, ny)) human_collision = true;
            } else if (!human_path.empty() && human_path.back() == make_pair(nx, ny)) {
                // Člověk už stojí v cíli
                human_collision = true;
            }
            if (human_collision) continue;

            // C) Je tam JINÝ ROBOT? (Collision Avoidance)
            bool robot_collision = false;
            for (const auto& [other_id, other_path] : current_paths) {
                if (other_id == agent_id) continue; // Ignoruji sám sebe

                // Kde je ten druhý robot v čase nt?
                pair<int, int> other_pos;
                if (nt < other_path.size()) {
                    other_pos = other_path[nt];
                } else {
                    other_pos = other_path.back(); // Už stojí v cíli
                }

                if (other_pos.first == nx && other_pos.second == ny) {
                    robot_collision = true;
                    break;
                }
            }
            if (robot_collision) continue;

            // KONEC KONTROL

            // Pokud je volno, přidáme do fronty
            int new_g = g_score[{cx, cy, ct}] + 1;
            tuple<int,int,int> next_state = {nx, ny, nt};

            if (g_score.find(next_state) == g_score.end() || new_g < g_score[next_state]) {
                g_score[next_state] = new_g;
                // Heuristika: vzdálenost k cíli
                int h = abs(nx - goal.first) + abs(ny - goal.second);
                open_set.push({nx, ny, nt, new_g + h});
                came_from[next_state] = {cx, cy, ct};
            }
        }
    }
    return nullopt; // Cesta nenalezena
}
