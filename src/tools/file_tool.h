#pragma once

#include "tool.h"
#include <string>

// FileTool: cho phép LLM đọc hoặc ghi nội dung 1 file trên máy.
// argsJson mong đợi dạng:
//   {"action": "read", "path": "result.txt"}
//   {"action": "write", "path": "result.txt", "content": "255"}
class FileTool : public Tool {
public:
    std::string getName() const override;
    std::string getDescription() const override;
    std::string execute(const std::string& argsJson) override;

private:
    std::string readFile(const std::string& path);
    std::string writeFile(const std::string& path, const std::string& content);
};