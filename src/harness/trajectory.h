#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Data class đóng gói chi tiết từng bước thực thi (Step)[cite: 1]
struct Step {
    int step_id{0};
    std::string thought;
    std::string action;
    std::string tool_result;
    long long latency_ms{0};
    int tokens_used{0};

    [[nodiscard]] json to_json() const;
};

// Data class đóng gói toàn bộ hành trình bài test (Trajectory)[cite: 1]
class Trajectory {
public:
    std::string task_id;
    std::string model;
    bool success{false};
    double score{0.0};
    int total_tokens{0};
    long long total_time_ms{0};
    std::vector<Step> steps;

    void add_step(const Step& step);
    [[nodiscard]] json to_json() const;
    void save_to_file(const std::string& filename) const;
};