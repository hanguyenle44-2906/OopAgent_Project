#include "agent.h"
#include <iostream>
#include <nlohmann/json.hpp>

// Tạo System Prompt ép LLM trả về đúng định dạng JSON
std::string Agent::buildSystemPrompt() {
    std::string prompt = "You are a helpful AI Agent. You have access to the following tools:\n\n";
    auto tools = registry_.listTools();
    for (const auto& [name, desc] : tools) {
        prompt += "- " + name + ": " + desc + "\n";
    }
    
    prompt += R"(
To use a tool, respond with ONLY a JSON object in this exact format:
{
    "thought": "your reasoning here",
    "action": "tool_name",
    "action_input": "{\"key\": \"value\"}" 
}

If you have reached the final answer, respond with ONLY:
{
    "thought": "i have the answer",
    "action": "final_answer",
    "action_input": "Your complete answer here"
}
)";
    return prompt;
}

std::string Agent::run(const std::string& userQuery) {
    std::vector<ChatMessage> messages;
    messages.push_back({"system", buildSystemPrompt()});
    messages.push_back({"user", userQuery});

    LLMConfig config; // Mặc định Ollama
    int step = 0;

    while (step < maxSteps_) {
        step++;
        std::cout << "\n--- [Step " << step << "] Calling LLM... ---" << std::endl;
        
        LLMResponse response;
        try {
            response = client_->generate(messages, config);
        } catch (const std::exception& e) {
            return std::string("LLM Error: ") + e.what();
        }

        std::string rawContent = response.content;
        std::cout << "LLM Raw Output:\n" << rawContent << std::endl;

        // Bóc tách JSON từ LLM
        nlohmann::json parsed;
        try {
            parsed = nlohmann::json::parse(rawContent);
        } catch (...) {
            // Nếu LLM không trả đúng JSON, nhồi lại message nhắc nhở
            messages.push_back({"assistant", rawContent});
            messages.push_back({"user", "Invalid JSON format. Please respond strictly in JSON format as specified."});
            continue;
        }

        std::string thought = parsed.value("thought", "");
        std::string action = parsed.value("action", "");
        
        // Kiểm tra câu trả lời cuối cùng
        if (action == "final_answer") {
            return parsed.value("action_input", rawContent);
        }

        // Ép kiểu action_input về chuỗi string JSON để truyền cho Tool
        std::string actionInputStr;
        if (parsed["action_input"].is_string()) {
            actionInputStr = parsed["action_input"].get<std::string>();
        } else {
            actionInputStr = parsed["action_input"].dump();
        }

        // Chặn Vòng Lặp (Loop Detector)
        std::string actionSignature = action + ":" + actionInputStr;
        if (loopDetector_.is_looping(actionSignature)) {
            std::cout << "[LoopDetector] Detected infinite loop on action: " << actionSignature << std::endl;
            return "Execution aborted: Agent got stuck in an infinite loop.";
        }

        // Thực thi Tool của Member A
        auto tool = registry_.createTool(action);
        std::string observation;
        if (!tool) {
            observation = "Error: Tool '" + action + "' is not available or blocked by policy.";
        } else {
            std::cout << "[Executing Tool]: " << action << " with args: " << actionInputStr << std::endl;
            observation = tool->execute(actionInputStr);
        }
        std::cout << "[Observation]: " << observation << std::endl;

        // Lưu ngữ cảnh lượt chạy vào cuộc hội thoại
        messages.push_back({"assistant", rawContent});
        messages.push_back({"user", "Observation: " + observation});
    }

    return "Max execution steps reached without a final answer.";
}