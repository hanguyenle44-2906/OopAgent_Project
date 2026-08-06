#include "harness_runner.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

HarnessRunner::HarnessRunner(Agent& agent) : agent_(agent) {}

double HarnessRunner::runSingleTask(const std::string& taskId, 
                                    const std::string& prompt, 
                                    const std::string& expected, 
                                    const std::string& evalType, 
                                    const std::string& evalScript) {
    
    // Thực thi Agent
    std::string result = agent_.run(prompt);

    // Áp dụng Strategy Pattern cho Evaluator
    std::unique_ptr<Evaluator> evaluator;
    if (evalType == "functional") {
        evaluator = std::make_unique<FunctionalEvaluator>();
        return evaluator->evaluate(result, evalScript);
    } else {
        evaluator = std::make_unique<KeywordEvaluator>();
        return evaluator->evaluate(result, expected);
    }
}

void HarnessRunner::runBenchmark(const std::string& tasksJsonPath, const std::string& outputTrajectoryPath) {
    std::ifstream inFile(tasksJsonPath);
    if (!inFile.is_open()) {
        std::cerr << "[HarnessRunner Error] Khong the mo file: " << tasksJsonPath << std::endl;
        return;
    }

    nlohmann::json tasks;
    inFile >> tasks;
    inFile.close();

    TrajectoryLogger logger;
    double totalScore = 0.0;
    int taskCount = tasks.size();

    std::cout << "\n=============================================" << std::endl;
    std::cout << "   BAT DAU CHAY BATCH BENCHMARK EVALUATION   " << std::endl;
    std::cout << "=============================================\n" << std::endl;

    for (const auto& task : tasks) {
        std::string id = task.value("id", "task_000");
        std::string prompt = task.value("instruction", task.value("prompt", ""));
        std::string expected = task.value("expected", "");
        std::string evalType = task.value("eval_type", "keyword");
        std::string evalScript = task.value("eval_script", "");

        std::cout << "[Task " << id << "] " << prompt << std::endl;
        
        double score = runSingleTask(id, prompt, expected, evalType, evalScript);
        totalScore += score;

        std::cout << " => Score: " << score << "/1.0\n" << std::endl;
        logger.logStep(std::stoi(id.substr(id.find_last_of('_') + 1)), prompt, "eval", "Score: " + std::to_string(score));
    }

    logger.saveToFile(outputTrajectoryPath);
    
    std::cout << "---------------------------------------------" << std::endl;
    std::cout << "Tỷ lệ thành công (Success Rate): " << (totalScore / taskCount) * 100.0 << "%" << std::endl;
    std::cout << "Trajectory log da duoc luu tai: " << outputTrajectoryPath << std::endl;
}