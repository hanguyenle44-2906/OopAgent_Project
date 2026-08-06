#include "evaluator.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>

// KeywordEvaluator Implementation
double KeywordEvaluator::evaluate(const std::string& actual_output, const std::string& expected_keyword) {
    if (expected_keyword.empty()) return 1.0;
    
    std::string actual_lower = actual_output;
    std::string expected_lower = expected_keyword;
    
    std::transform(actual_lower.begin(), actual_lower.end(), actual_lower.begin(), ::tolower);
    std::transform(expected_lower.begin(), expected_lower.end(), expected_lower.begin(), ::tolower);

    if (actual_lower.find(expected_lower) != std::string::npos) {
        return 1.0;
    }
    return 0.0;
}

// FunctionalEvaluator Implementation
double FunctionalEvaluator::evaluate(const std::string& actual_output, const std::string& expected_condition) {
    if (expected_condition.empty()) return 1.0;

    // Chạy câu lệnh shell kiểm tra tính năng (vd: grep/test file)
    int result = std::system(expected_condition.c_str());
    if (result == 0) {
        return 1.0;
    }
    
    // Fallback kiểm tra chuỗi nếu không phải lệnh shell
    return (actual_output.find(expected_condition) != std::string::npos) ? 1.0 : 0.0;
}