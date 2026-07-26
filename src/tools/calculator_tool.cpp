#include "calculator_tool.h"
#include <nlohmann/json.hpp>
#include <cctype>
#include <stdexcept>

std::string CalculatorTool::getName() const {
    return "calculator";
}

std::string CalculatorTool::getDescription() const {
    return "Tinh toan bieu thuc so hoc. Tham so: {\"expression\": \"2 + 3 * 4\"}";
}

std::string CalculatorTool::execute(const std::string& argsJson) {
    try {
        nlohmann::json args = nlohmann::json::parse(argsJson);
        expr_ = args.value("expression", "");
        pos_ = 0;
        double result = parseExpression();
        return std::to_string(result);
    }
    catch (const std::exception& e) {
        return std::string("Loi: ") + e.what();
    }
}

void CalculatorTool::skipSpaces() {
    while (pos_ < expr_.size() && expr_[pos_] == ' ') pos_++;
}

double CalculatorTool::parseExpression() {
    double value = parseTerm();
    skipSpaces();
    while (pos_ < expr_.size() && (expr_[pos_] == '+' || expr_[pos_] == '-')) {
        char op = expr_[pos_++];
        double rhs = parseTerm();
        value = (op == '+') ? value + rhs : value - rhs;
        skipSpaces();
    }
    return value;
}

double CalculatorTool::parseTerm() {
    double value = parseFactor();
    skipSpaces();
    while (pos_ < expr_.size() && (expr_[pos_] == '*' || expr_[pos_] == '/')) {
        char op = expr_[pos_++];
        double rhs = parseFactor();
        if (op == '*') {
            value *= rhs;
        }
        else {
            if (rhs == 0) throw std::runtime_error("Chia cho 0");
            value /= rhs;
        }
        skipSpaces();
    }
    return value;
}

double CalculatorTool::parseFactor() {
    skipSpaces();
    if (pos_ < expr_.size() && expr_[pos_] == '(') {
        pos_++;
        double value = parseExpression();
        skipSpaces();
        if (pos_ < expr_.size() && expr_[pos_] == ')') pos_++;
        return value;
    }
    size_t start = pos_;
    if (pos_ < expr_.size() && expr_[pos_] == '-') pos_++;
    while (pos_ < expr_.size() && (isdigit(expr_[pos_]) || expr_[pos_] == '.')) {
        pos_++;
    }
    if (start == pos_) {
        throw std::runtime_error("Bieu thuc khong hop le tai vi tri " + std::to_string(pos_));
    }
    return std::stod(expr_.substr(start, pos_ - start));
}