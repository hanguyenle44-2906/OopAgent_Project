#include "src/tools/tool_registry.h"
#include "src/tools/calculator_tool.h"
#include "src/tools/file_tool.h"
#include <iostream>

int main() {
    ToolRegistry registry;
    registry.registerTool("calculator", []() { return std::make_unique<CalculatorTool>(); });
    registry.registerTool("file", []() { return std::make_unique<FileTool>(); });

    for (const auto& [name, desc] : registry.listTools()) {
        std::cout << "Tool: " << name << " - " << desc << "\n";
    }

    auto fileTool = registry.createTool("file");
    std::string writeResult = fileTool->execute(
        R"({"action":"write","path":"result.txt","content":"255"})");
    std::cout << "Ghi: " << writeResult << "\n";

    std::string readResult = fileTool->execute(
        R"({"action":"read","path":"result.txt"})");
    std::cout << "Doc lai: " << readResult << "\n";

    return 0;
}