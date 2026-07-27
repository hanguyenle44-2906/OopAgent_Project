#include "src/tools/tool_registry.h"
#include "src/tools/calculator_tool.h"
#include "src/tools/file_tool.h"
#include "src/tools/exec_tool.h"

#include "src/client/ollama_client.h"
#include "src/agent/agent.h"
#include "src/harness/evaluator.h"
#include "src/harness/trajectory_logger.h"

#include <iostream>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>

int main() {
    // Khởi tạo ToolRegistry & đăng ký các Tool 
    ToolRegistry registry;

    registry.registerTool("calculator", []() { return std::make_unique<CalculatorTool>(); });
    registry.registerTool("file", []() { return std::make_unique<FileTool>(); });
    registry.registerTool("exec", []() { return std::make_unique<ExecTool>(); });

    auto execTool = registry.createTool("exec");
    std::string output = execTool->execute(R"({"command": "echo Hello from exec tool"})");
    std::cout << "Exec output: " << output << std::endl;

    auto llmClient = std::make_shared<OllamaClient>();
    Agent agent(llmClient, registry, 5); // Tối đa 5 bước suy luận cho mỗi task
    KeywordEvaluator evaluator;
    TrajectoryLogger logger;

    // 2. Đọc tập Benchmark từ file tasks.json
    std::ifstream tasksFile("tasks.json");
    if (!tasksFile.is_open()) {
        std::cerr << "[Error] Khong the mo file tasks.json!" << std::endl;
        return 1;
    }

    nlohmann::json tasks;
    tasksFile >> tasks;
    tasksFile.close();

    double totalScore = 0.0;
    int taskCount = tasks.size();

    // 3. Chạy từng task trong benchmark
    for (const auto& task : tasks) {
        int id = task.value("id", 0);
        std::string prompt = task.value("prompt", "");
        std::string expected = task.value("expected", "");

        std::cout << "\n------------------------------------------" << std::endl;
        std::cout << "[Task " << id << "] Prompt: " << prompt << std::endl;
        std::cout << "[Expected]: " << expected << std::endl;

        // Chạy Agent suy luận ReAct Loop
        std::string result = agent.run(prompt);
        std::cout << "[Agent Output]: " << result << std::endl;

        // Tự động chấm điểm
        double score = evaluator.evaluate(result, expected);
        totalScore += score;
        std::cout << "[Score]: " << score << "/1.0" << std::endl;

        // Ghi Trajectory log
        logger.logStep(id, prompt, "run_task", result);
    }

    logger.saveToFile("trajectory_log.json");

    return 0;
}