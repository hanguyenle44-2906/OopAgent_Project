#pragma once

#include "tool.h"
#include <string>

// WebTool: tim kiem thong tin qua DuckDuckGo Instant Answer API
// (khong can API key). Tham so: {"query": "thu do cua Viet Nam"}
class WebTool : public Tool {
public:
    std::string getName() const override;
    std::string getDescription() const override;
    std::string execute(const std::string& argsJson) override;
};