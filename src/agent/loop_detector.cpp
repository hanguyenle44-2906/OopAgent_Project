#include "loop_detector.h"

// Định nghĩa phương thức kiểm tra lặp
bool LoopDetector::detectLoop(const std::string& action, const std::string& argsJson) {
    std::string key = action + "::" + argsJson;
    history_.push_back(key);
    actionCounts_[key]++;

    // 1. Generic Repeat Detection (Action + Args trùng quá ngưỡng)
    if (actionCounts_[key] >= maxRepeatThreshold_) {
        return true;
    }

    // 2. Ping-Pong Loop Detection (A -> B -> A -> B)
    size_t n = history_.size();
    if (n >= 4) {
        if (history_[n - 1] == history_[n - 3] && history_[n - 2] == history_[n - 4]) {
            return true;
        }
    }
    return false;
}

void LoopDetector::reset() {
    history_.clear();
    actionCounts_.clear();
}