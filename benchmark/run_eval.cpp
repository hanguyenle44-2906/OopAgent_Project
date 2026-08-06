#include "../src/tools/tool_registry.h"
#include "../src/tools/calculator_tool.h"
#include "../src/tools/file_tool.h"
#include "../src/tools/exec_tool.h"
#include "../src/tools/web_tool.h"
#include "../src/tools/memory_tool.h"

#include "../src/client/ollama_client.h"
#include "../src/agent/agent.h"
#include "../src/harness/harness_runner.h"

#include <iostream>
#include <memory>

int main() {
    // Khởi tạo Registry & Đăng ký Tools
    ToolRegistry registry;
    registry.registerTool("calculator", []() { return std::make_unique<CalculatorTool>(); });
    registry.registerTool("file", []() { return std::make_unique<FileTool>(); });
    registry.registerTool("exec", []() { return std::make_unique<ExecTool>(); });
    registry.registerTool("web_search", []() { return std::make_unique<WebTool>(); });
    registry.registerTool("memory", []() { return std::make_unique<MemoryTool>(); });

    // Khởi tạo Client & Agent Loop
    auto llmClient = std::make_shared<OllamaClient>();
    Agent agent(llmClient, registry, 10);

    // Khởi tạo Harness Runner để đánh giá Benchmark
    HarnessRunner runner(agent);
    
    // Chạy batch evaluation tập benchmark tasks.json
    runner.runBenchmark("tasks.json", "trajectory_batch_log.json");

    return 0;
}