#pragma once
// #pragma once: báo cho compiler "chỉ đọc file này 1 lần duy nhất", dù có
// nhiều file .cpp khác nhau #include file này. Không có dòng này, nếu 2 file
// cùng include llm_client.h thì compiler sẽ định nghĩa trùng lặp và báo lỗi.

#include <string>
#include <vector>
#include <optional>

// ===== struct: các "gói dữ liệu" dùng chung giữa LLMClient và AgentLoop =====

// Cấu hình để gọi tới LLM: model nào, ở đâu, độ "sáng tạo" bao nhiêu.
struct LLMConfig {
    std::string baseUrl = "http://localhost:11434";
    std::string model = "qwen3-vl:4b";
    double temperature = 0.7;
    int maxTokens = 2048;
};

// Một tin nhắn trong cuộc hội thoại gửi cho LLM.
// role thường là "system" (chỉ dẫn ban đầu), "user" (câu hỏi), hoặc
// "assistant" (câu trả lời trước đó của LLM, dùng khi cần giữ ngữ cảnh).
struct ChatMessage {
    std::string role;
    std::string content;
    // optional vì không phải tin nhắn nào cũng kèm ảnh (multimodal).
    // Mỗi ảnh lưu dạng chuỗi base64 (cách mã hoá dữ liệu nhị phân thành text).
    std::optional<std::vector<std::string>> images;
};

// Kết quả trả về sau khi gọi LLM xong.
struct LLMResponse {
    std::string content;
    int promptTokens = 0;
    int completionTokens = 0;
    long long latencyMs = 0;
};

// ===== Interface (abstract class) =====
// AgentLoop sẽ chỉ cầm 1 con trỏ tới LLMClient, không biết (và không cần
// biết) bên trong là OllamaClient hay bất kỳ client nào khác.
class LLMClient {
public:
    // Destructor ảo (virtual): BẮT BUỘC phải có khi 1 class dự định làm
    // interface để class khác kế thừa. Nếu thiếu, khi xoá object qua con trỏ
    // LLMClient*, bộ nhớ của phần OllamaClient có thể không được giải phóng
    // đúng cách (rò rỉ bộ nhớ - memory leak).
    virtual ~LLMClient() = default;

    // "= 0" ở cuối là điều làm cho đây thành pure virtual function.
    // Class con (OllamaClient) BẮT BUỘC phải viết lại hàm này, nếu không
    // compiler sẽ báo lỗi.
    virtual LLMResponse generate(const std::vector<ChatMessage>& messages,
        const LLMConfig& config) = 0;
};