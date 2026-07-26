#pragma once

#include "tool.h"
#include <string>

// ExecTool: chay 1 lenh shell va tra ve output (stdout).
// argsJson mong doi dang: {"command": "echo hello"}
class ExecTool : public Tool {
public:
    std::string getName() const override;
    std::string getDescription() const override;
    std::string execute(const std::string& argsJson) override;
};