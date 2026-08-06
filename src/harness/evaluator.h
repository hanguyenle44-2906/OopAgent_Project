#pragma once

#include <string>
#include <algorithm>
#include <cstdlib> // Thêm thư viện này cho std::system

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

// Chấm điểm chức năng / chạy test script tự động
class FunctionalEvaluator : public Evaluator {
public:
    double evaluate(const std::string& actualOutput, const std::string& expectedOutput) override {
        if (expectedOutput.empty()) {
            return actualOutput.empty() ? 0.0 : 1.0;
        }
        // Nếu expectedOutput chứa lệnh/script kiểm thử thì thực thi lệnh hệ thống
        int result = std::system(expectedOutput.c_str());
        return (result == 0) ? 1.0 : 0.0;
    }
};