#pragma once

#include <string>
#include <algorithm>

// Interface chung cho các bộ chấm điểm (Observer Pattern)
class Evaluator {
public:
    virtual ~Evaluator() = default;
    virtual double evaluate(const std::string& actualOutput, const std::string& expectedOutput) = 0;
};

// Chấm điểm dựa trên việc từ khóa/đáp án có xuất hiện trong câu trả lời không
class KeywordEvaluator : public Evaluator {
public:
    double evaluate(const std::string& actualOutput, const std::string& expectedOutput) override {
        if (actualOutput.empty() || expectedOutput.empty()) return 0.0;
        
        // Chuyển về chữ thường để so sánh không phân biệt hoa/thường
        std::string actualLower = actualOutput;
        std::string expectedLower = expectedOutput;
        std::transform(actualLower.begin(), actualLower.end(), actualLower.begin(), ::tolower);
        std::transform(expectedLower.begin(), expectedLower.end(), expectedLower.begin(), ::tolower);

        return (actualLower.find(expectedLower) != std::string::npos) ? 1.0 : 0.0;
    }
};