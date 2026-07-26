#include "src/tools/tool_registry.h"
#include "src/tools/calculator_tool.h"
#include <iostream>

int main() {
    ToolRegistry registry;
    registry.registerTool("calculator", []() {
        return std::make_unique<CalculatorTool>();
        });

    // In danh sách tool đã đăng ký (giả lập việc build system prompt)
    for (const auto& [name, desc] : registry.listTools()) {
        std::cout << "Tool: " << name << " - " << desc << "\n";
    }

    // Tạo thử 1 tool và gọi execute
    auto tool = registry.createTool("calculator");
    if (tool) {
        std::string result = tool->execute(R"({"expression": "2 + 3 * 4"})");
        std::cout << "Ket qua: " << result << "\n";
    }

    return 0;
}