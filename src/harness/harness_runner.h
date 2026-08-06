#pragma once

#include "../agent/agent.h"
#include "evaluator.h"
#include "trajectory_logger.h"
#include <string>
#include <vector>

class HarnessRunner {
public:
    explicit HarnessRunner(Agent& agent);

    // Chạy 1 task lẻ và trả về điểm số
    double runSingleTask(const std::string& taskId, 
                         const std::string& prompt, 
                         const std::string& expected, 
                         const std::string& evalType, 
                         const std::string& evalScript);

    // Batch Evaluation: Chạy toàn bộ file benchmark tasks.json và xuất Trajectory
    void runBenchmark(const std::string& tasksJsonPath, const std::string& outputTrajectoryPath);

private:
    Agent& agent_;
};