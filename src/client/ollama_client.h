#pragma once

#include "llm_client.h"
#include <stdexcept>

// Exception riêng cho lỗi liên quan tới LLMClient (mạng lỗi, JSON hỏng...).
// Kế thừa std::runtime_error (có sẵn trong thư viện chuẩn) để AgentLoop có
// thể "catch" đúng loại lỗi này riêng biệt, khác với lỗi khác trong hệ thống.
class LLMClientException : public std::runtime_error {
public:
    explicit LLMClientException(const std::string& message)
        : std::runtime_error(message) {}
};

// Class CỤ THỂ (concrete class) - class này mới thực sự tạo object được,
// vì nó viết đầy đủ thân hàm cho generate() (không còn "= 0" nữa).
class OllamaClient : public LLMClient {
public:
    // override: từ khóa báo cho compiler biết "đây là tôi đang viết lại 1
    // hàm ảo của lớp cha". Không bắt buộc phải viết, nhưng NÊN viết luôn:
    // nếu bạn gõ sai tên hàm (ví dụ "generete" thay vì "generate"), compiler
    // sẽ báo lỗi ngay thay vì âm thầm tạo ra 1 hàm mới không liên quan gì
    // tới interface LLMClient - lỗi kiểu này rất khó phát hiện nếu thiếu
    // override.
    LLMResponse generate(const std::vector<ChatMessage>& messages,
        const LLMConfig& config) override;
};