// test_memory.cpp - CHỈ ĐỂ THAM KHẢO, không add vào CMakeLists, xoá sau khi test xong
#include "src/tools/tool_registry.h"
#include "src/tools/memory_tool.h"
#include <iostream>

int main() {
    ToolRegistry registry;
    registry.registerTool("memory", []() { return std::make_unique<MemoryTool>(); });

    auto memTool = registry.createTool("memory");
    std::cout << memTool->execute(R"({"action":"save","key":"mon_hoc","content":"OOP la mon hoc thu vi"})") << "\n";
    std::cout << memTool->execute(R"({"action":"search","query":"OOP"})");

    return 0;
}