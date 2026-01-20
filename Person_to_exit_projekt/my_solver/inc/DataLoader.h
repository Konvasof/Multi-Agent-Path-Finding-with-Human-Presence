#pragma once
#include <string>
#include <map>
#include <vector>
#include <set>
#include <utility>

class LogData {
public:
    // Mapa: čas -> seznam překážek (agentů)
    std::map<int, std::vector<std::pair<int, int>>> obstacles_at_time;
    // Lookup: (čas, x, y) -> Agent ID
    std::map<std::tuple<int, int, int>, int> agent_lookup;

    explicit LogData(const std::string& raw_log_content);

    std::set<std::pair<int, int>> get_obstacles(int time_t);
    int get_max_time() const;
    int get_agent_at(int time, std::pair<int, int> pos);

private:
    void _parse_data(const std::string& raw_data);
};
