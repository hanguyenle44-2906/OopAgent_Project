#include "agent.h"
#include <iostream>
#include <nlohmann/json.hpp>

// ============================================================================
// HELPER: cleanJsonResponse()
// ----------------------------------------------------------------------------
// Mục đích: Tự động trích xuất chuỗi JSON nằm giữa Markdown block (```json ... ```)
// hoặc cắt bỏ các ký tự thừa trước { và sau }, giúp nlohmann::json parse 100% 
// thành công kể cả khi LLM kèm thêm text tự do.
// ============================================================================
static std::string cleanJsonResponse(const std::string& raw) {
    std::string s = raw;
    
    // 1. Loại bỏ Markdown codeblocks (```json ... ```)
    std::size_t startPos = s.find("```json");
    if (startPos != std::string::npos) {
        s.erase(startPos, 7);
    } else {
        startPos = s.find("```");
        if (startPos != std::string::npos) {
            s.erase(startPos, 3);
        }
    }
    std::size_t endPos = s.rfind("```");
    if (endPos != std::string::npos) {
        s.erase(endPos, 3);
    }

    // 2. Cắt các ký tự trắng thừa ở 2 đầu
    size_t first = s.find('{');
    size_t last = s.rfind('}');
    if (first != std::string::npos && last != std::string::npos && last >= first) {
        return s.substr(first, (last - first + 1));
    }
    
    return s;
}

// buildSystemPrompt()
// ----------------------------------------------------------------------------
// Mục đích: Tạo System Prompt để "ép" LLM luôn phản hồi theo đúng định dạng
// JSON chuẩn ReAct (Reasoning + Acting): thought -> action -> action_input.
// Nhờ đó Agent (C++) có thể parse output của LLM một cách an toàn thay vì
// phải xử lý văn bản tự do (free-text).
// ============================================================================
std::string Agent::buildSystemPrompt() {
    // Đoạn mở đầu: giới thiệu vai trò của LLM và liệt kê danh sách tool khả dụng
    std::string prompt = "You are a helpful AI Agent. You have access to the following tools:\n\n";

    // Lấy danh sách tool đã đăng ký trong ToolRegistry (do Member A cung cấp)
    // Mỗi tool gồm: tên (name) và mô tả chức năng (desc) để LLM biết khi nào nên dùng
    auto tools = registry_.listTools();
    for (const auto& [name, desc] : tools) {
        prompt += "- " + name + ": " + desc + "\n";
    }

    // Đoạn hướng dẫn định dạng bắt buộc (Format Enforcement):
    // - Nếu muốn gọi tool: trả về JSON với action = tên tool, action_input = tham số (dạng chuỗi JSON)
    // - Nếu đã có câu trả lời cuối: action = "final_answer"
    // Việc yêu cầu "respond with ONLY a JSON object" giúp giảm thiểu rủi ro LLM
    // chèn thêm text giải thích ngoài JSON, gây lỗi parse ở bước sau.
    prompt += R"(
CRITICAL TOOL SCHEMA RULES:
1. For tool "exec", "action_input" MUST be a JSON object string: "{\"command\": \"your_command_here\"}"
2. For tool "file", "action_input" MUST be a JSON object string with "path" and "content" or "mode".
3. Do NOT invent non-existent tools (e.g. "print"). Use "final_answer" when done.

To use a tool, respond with ONLY a JSON object in this exact format:
{
    "thought": "your reasoning here",
    "action": "tool_name",
    "action_input": "{\"command\": \"echo Hello\"}" 
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

// ============================================================================
// run()
// ----------------------------------------------------------------------------
// Mục đích: Vòng lặp chính của Agent theo mô hình ReAct:
//   1) Gửi lịch sử hội thoại (kèm system prompt) cho LLM
//   2) LLM trả về JSON gồm thought/action/action_input
//   3) Nếu action = final_answer -> trả kết quả cho user, kết thúc
//   4) Nếu action = tên tool -> thực thi tool, lấy Observation, đưa Observation
//      trở lại vào hội thoại để LLM "suy nghĩ" tiếp ở bước sau
//   5) Lặp lại tối đa maxSteps_ bước, có cơ chế chống lặp vô hạn (LoopDetector)
// ============================================================================
std::string Agent::run(const std::string& userQuery) {
    // ---- Khởi tạo lịch sử hội thoại gửi cho LLM ----
    // messages đóng vai trò "bộ nhớ ngắn hạn" của Agent trong 1 lượt chạy (run)
    std::vector<ChatMessage> messages;
    messages.push_back({"system", buildSystemPrompt()});   // Prompt hệ thống: quy tắc + danh sách tool
    messages.push_back({"user", userQuery});               // Câu hỏi/gốc yêu cầu ban đầu của user

    LLMConfig config; // Cấu hình mặc định gọi LLM (mặc định trỏ tới Ollama, theo comment gốc)
    
    // [GIA CỐ]: Thiết lập thông số an toàn cho config
    config.model = "qwen2.5:0.5b";
    config.temperature = 0.1;

    int step = 0;     // Đếm số bước (vòng lặp) đã thực hiện, để giới hạn bởi maxSteps_

    // ---- Vòng lặp ReAct chính ----
    while (step < maxSteps_) {
        step++;
        std::cout << "\n--- [Step " << step << "] Calling LLM... ---" << std::endl;

        // (1) Gọi LLM với toàn bộ lịch sử hội thoại hiện tại
        LLMResponse response;
        try {
            response = client_->generate(messages, config);
        } catch (const std::exception& e) {
            // [GIA CỐ]: In thông báo lỗi trực tiếp ra Terminal để dễ debug
            std::cout << "[Agent Error] Exception encountered: " << e.what() << std::endl;
            
            // Nếu lỗi kết nối/timeout tới LLM -> dừng ngay, trả lỗi cho user
            // (không retry ở đây để tránh vòng lặp tốn tài nguyên khi LLM down)
            return std::string("LLM Error: ") + e.what();
        }

        std::string rawContent = response.content;
        std::cout << "LLM Raw Output:\n" << rawContent << std::endl;

        // (2) Bóc tách (parse) JSON từ nội dung LLM trả về
        nlohmann::json parsed;
        try {
            // [THÊM GIA CỐ]: Làm sạch chuỗi trước khi parse
            std::string cleanedContent = cleanJsonResponse(rawContent);
            parsed = nlohmann::json::parse(cleanedContent);
        } catch (...) {
            // Trường hợp LLM "phá lệ" không trả đúng JSON (rất hay gặp với model nhỏ):
            // - Lưu lại output sai định dạng vào lịch sử (để LLM thấy được lỗi của chính nó)
            // - Gửi thêm message nhắc nhở, yêu cầu trả lời lại đúng format
            // - continue: bỏ qua phần xử lý bên dưới, quay lại đầu vòng lặp gọi LLM lần nữa
            // Lưu ý: bước này KHÔNG tăng thêm giới hạn riêng, vẫn tính vào maxSteps_,
            // nên nếu LLM liên tục sai định dạng, vòng lặp sẽ tự dừng khi hết step.
            messages.push_back({"assistant", rawContent});
            messages.push_back({"user", "Invalid JSON format. Please respond strictly in JSON format as specified."});
            continue;
        }

        // (3) Lấy các trường cần thiết từ JSON đã parse
        // Dùng value(key, default) để tránh crash nếu thiếu field (an toàn hơn parsed["thought"])
        std::string thought = parsed.value("thought", "");
        std::string action = parsed.value("action", "");

        // (4) Kiểm tra điều kiện dừng: nếu LLM đã có câu trả lời cuối cùng
        if (action == "final_answer") {
            // Trả thẳng action_input làm kết quả cuối; nếu thiếu field thì fallback về rawContent
            return parsed.value("action_input", rawContent);
        }

        // (5) Chuẩn hóa action_input về dạng std::string để truyền cho Tool::execute()
        // Vì LLM có thể trả action_input dưới 2 dạng:
        //   - Chuỗi JSON đã escape sẵn: "action_input": "{\"key\": \"value\"}"
        //   - Object JSON thật sự:      "action_input": {"key": "value"}
        // -> Cần đồng nhất về string trước khi đưa cho Tool xử lý (Tool tự parse JSON bên trong)
        std::string actionInputStr;
        if (parsed["action_input"].is_string()) {
            actionInputStr = parsed["action_input"].get<std::string>();
        } else {
            actionInputStr = parsed["action_input"].dump(); // ép object/array... thành chuỗi JSON
        }

        // (6) Cơ chế chống lặp vô hạn (Loop Detector)
        // Tạo "chữ ký" của hành động = tên tool + tham số đầu vào.
        // Nếu Agent lặp lại y hệt 1 action nhiều lần liên tiếp (LLM bị kẹt, không tiến triển),
        // LoopDetector sẽ phát hiện và Agent chủ động dừng thay vì chạy tới hết maxSteps_.
        std::string actionSignature = action + ":" + actionInputStr;
        if (loopDetector_.detectLoop(action, actionInputStr)) {
            std::cout << "[LoopDetector] Detected infinite loop on action: " << actionSignature << std::endl;
            return "Execution aborted: Agent got stuck in an infinite loop.";
        }

        // (7) Thực thi Tool tương ứng (Tool do Member A cung cấp qua ToolRegistry)
        auto tool = registry_.createTool(action);
        std::string observation;
        if (!tool) {
            // Tool không tồn tại hoặc bị chặn bởi policy (ví dụ: whitelist/blacklist)
            // -> không throw exception mà trả về observation dạng lỗi để LLM tự biết
            //    và có thể điều chỉnh hành động ở bước tiếp theo (self-correction)
            observation = "Error: Tool '" + action + "' is not available or blocked by policy.";
        } else {
            std::cout << "[Executing Tool]: " << action << " with args: " << actionInputStr << std::endl;
            observation = tool->execute(actionInputStr);
        }
        std::cout << "[Observation]: " << observation << std::endl;

        // (8) Cập nhật lịch sử hội thoại với:
        //   - Chính output gốc của LLM (đóng vai "assistant") để giữ ngữ cảnh reasoning
        //   - Observation (kết quả thực thi tool) đóng vai "user" gửi lại cho LLM,
        //     để LLM dùng thông tin này suy luận tiếp ở vòng lặp kế
        messages.push_back({"assistant", rawContent});
        messages.push_back({"user", "Observation: " + observation});

        // Kết thúc 1 vòng lặp -> quay lại while, tiếp tục gọi LLM với ngữ cảnh mới
    }

    // (9) Nếu chạy hết số bước cho phép (maxSteps_) mà LLM vẫn chưa trả final_answer
    // -> coi như Agent không hoàn thành nhiệm vụ trong giới hạn cho phép
    return "Max execution steps reached without a final answer.";
}