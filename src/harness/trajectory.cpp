#include "trajectory.h"
#include <fstream>
#include <iostream>

json Step::to_json() const {
    return json{
        {"step_id", step_id},
        {"thought", thought},
        {"action", action},
        {"tool_result", tool_result},
        {"tokens_used", tokens_used},
        {"latency_ms", latency_ms}
    };
}

void Trajectory::add_step(const Step& step) {
    steps.push_back(step);
}

json Trajectory::to_json() const {
    json j_steps = json::array();
    for (const auto& s : steps) {
        j_steps.push_back(s.to_json());
    }

    return json{
        {"task_id", task_id},
        {"model", model},
        {"success", success},
        {"score", score},
        {"total_tokens", total_tokens},
        {"total_time_ms", total_time_ms},
        {"steps", j_steps}
    };
}

void Trajectory::save_to_file(const std::string& filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << to_json().dump(2);
        file.close();
        std::cout << "[Trajectory] Log saved to " << filename << "\n";
    } else {
        std::cerr << "[Trajectory Error] Cannot open file " << filename << " to write.\n";
    }
}