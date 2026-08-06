#pragma once
#include <string>
#include <vector>
#include <unordered_map>

class LoopDetector {
public:
    LoopDetector(int maxRepeatThreshold = 3) : maxRepeatThreshold_(maxRepeatThreshold) {}

    bool detectLoop(const std::string& action, const std::string& argsJson);
    void reset();

private:
    int maxRepeatThreshold_;
    std::vector<std::string> history_;
    std::unordered_map<std::string, int> actionCounts_;
};