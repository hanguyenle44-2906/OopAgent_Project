#pragma once

#include <string>

// Interface chung cho các bộ chấm điểm (Strategy Pattern)
class Evaluator {
public:
    virtual ~Evaluator() = default;
    virtual double evaluate(const std::string& actual_output, const std::string& expected) = 0;
};

// Chấm điểm dựa trên từ khóa
class KeywordEvaluator : public Evaluator {
public:
    double evaluate(const std::string& actual_output, const std::string& expected_keyword) override;
};

// Chấm điểm chức năng / chạy test script tự động
class FunctionalEvaluator : public Evaluator {
public:
    double evaluate(const std::string& actual_output, const std::string& expected_condition) override;
};