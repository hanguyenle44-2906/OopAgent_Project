#include "src/tools/tool_registry.h"
#include "src/tools/calculator_tool.h"
#include "src/tools/file_tool.h"
#include "src/tools/exec_tool.h"
#include <iostream>

int main() {
    ToolRegistry registry;

    registry.registerTool("calculator", []() { return std::make_unique<CalculatorTool>(); });
    registry.registerTool("file", []() { return std::make_unique<FileTool>(); });
    registry.registerTool("exec", []() { return std::make_unique<ExecTool>(); });

    auto execTool = registry.createTool("exec");
    std::string output = execTool->execute(R"({"command": "echo Hello from exec tool"})");
    std::cout << "Exec output: " << output;

    return 0;
}