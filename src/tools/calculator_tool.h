#pragma once

#include "tool.h"
#include <string>

class CalculatorTool : public Tool {
public:
    std::string getName() const override;
    std::string getDescription() const override;
    std::string execute(const std::string& argsJson) override;

private:
    double parseExpression();
    double parseTerm();
    double parseFactor();
    void skipSpaces();

    std::string expr_;
    size_t pos_ = 0;
};