#pragma once

#include "../client/llm_client.h"
#include "../tools/tool_registry.h"
#include "loop_detector.h"
#include <memory>
#include <string>
#include <vector>

class Agent {
private:
    std::shared_ptr<LLMClient> client_;
    ToolRegistry& registry_;
    LoopDetector loopDetector_;
    int maxSteps_;

    std::string buildSystemPrompt();

public:
    Agent(std::shared_ptr<LLMClient> client, ToolRegistry& registry, int maxSteps = 10)
        : client_(client), registry_(registry), maxSteps_(maxSteps), loopDetector_(3) {}

    std::string run(const std::string& userQuery);
};
