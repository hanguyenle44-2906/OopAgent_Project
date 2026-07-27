#pragma once

#include <string>
#include <unordered_map>

class LoopDetector {
private:
    int maxRepeats_;
    std::unordered_map<std::string, int> history_;

public:
    explicit LoopDetector(int maxRepeats = 3) : maxRepeats_(maxRepeats) {}

    // Đổi tên hoặc thêm hàm is_looping để khớp với agent.cpp
    bool is_looping(const std::string& action) {
        history_[action]++;
        return history_[action] >= maxRepeats_;
    }

    bool isLoop(const std::string& action) {
        return is_looping(action);
    }

    void clear() {
        history_.clear();
    }
};
