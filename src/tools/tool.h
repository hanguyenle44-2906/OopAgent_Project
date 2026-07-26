#pragma once

#include <string>

// Interface cho MỌI tool. Giống hệt tinh thần của LLMClient: AgentLoop chỉ
// biết làm việc qua Tool* (con trỏ tới interface), không cần biết bên dưới
// là CalculatorTool hay FileTool hay gì khác.
class Tool {
public:
    virtual ~Tool() = default;

    // Tên định danh, duy nhất, dùng làm "chìa khoá" trong ToolRegistry.
    // Đây cũng chính là cái tên mà LLM sẽ nhìn thấy khi được liệt kê danh
    // sách tool, và là tên LLM sẽ "gọi" khi muốn dùng tool này.
    virtual std::string getName() const = 0;

    // Mô tả ngắn gọn, được đưa vào system prompt để LLM biết tool này dùng
    // để làm gì và dùng lúc nào - viết mô tả càng rõ, LLM càng gọi tool
    // đúng lúc hơn (đây là 1 dạng "prompt engineering" gián tiếp).
    virtual std::string getDescription() const = 0;

    // argsJson: tham số LLM truyền vào, dạng chuỗi JSON (ví dụ tool
    // calculator sẽ nhận vào {"expression": "2 + 3 * 4"}).
    // Trả về: kết quả dạng string, sẽ được đưa ngược lại vào prompt cho LLM
    // đọc ở bước "quan sát" (observe) tiếp theo trong vòng lặp ReAct.
    //
    // "const" ở cuối khai báo hàm: cam kết hàm này KHÔNG thay đổi bất kỳ
    // field nào của chính object Tool đó (không đổi tên, mô tả...). Không
    // bắt buộc phải const cho execute() vì 1 số tool có thể cần lưu trạng
    // thái (ví dụ MemoryTool giữ kết nối database) - nên execute() ở đây
    // KHÔNG const, khác với getName()/getDescription() ở trên.
    virtual std::string execute(const std::string& argsJson) = 0;
};