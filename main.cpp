#include "src/tools/tool_registry.h"
#include "src/tools/calculator_tool.h"
#include "src/tools/file_tool.h"
#include "src/tools/exec_tool.h"
#include "src/tools/web_tool.h"
#include <iostream>

int main() {
    ToolRegistry registry;

    registry.registerTool("calculator", []() { return std::make_unique<CalculatorTool>(); });
    registry.registerTool("file", []() { return std::make_unique<FileTool>(); });
    registry.registerTool("exec", []() { return std::make_unique<ExecTool>(); });
    registry.registerTool("web_search", []() { return std::make_unique<WebTool>(); });

    auto searchTool = registry.createTool("web_search");
    std::string result = searchTool->execute(R"({"query": "Vietnam"})");
    std::cout << "Ket qua tim kiem: " << result << "\n";

    return 0;
}