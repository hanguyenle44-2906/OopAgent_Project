#include "ollama_client.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <chrono>

// ===== Phần libcurl: giải thích trước khi đọc code =====
//
// libcurl là thư viện C thuần (không phải C++), nên cách dùng nó hơi khác
// "phong cách" các thư viện bạn quen (như <vector>, <string>). Quy trình
// chuẩn để gửi 1 request bằng libcurl luôn gồm các bước sau, không đổi:
//
//   1. curl_easy_init()       -> tạo 1 "handle" (tay cầm), đại diện cho
//                                 1 lần gọi request. Trả về con trỏ CURL*.
//   2. curl_easy_setopt(...)  -> cấu hình handle đó: URL nào, method gì
//                                 (GET/POST), header gì, dữ liệu gửi kèm...
//                                 Gọi hàm này nhiều lần, mỗi lần set 1 tuỳ
//                                 chọn (option) khác nhau.
//   3. curl_easy_perform()    -> THỰC SỰ gửi request đi và đợi phản hồi.
//                                 Đây là dòng lệnh duy nhất "tốn thời gian"
//                                 (blocking - chương trình dừng lại đợi).
//   4. curl_easy_cleanup()    -> giải phóng handle, giống như "dọn dẹp" sau
//                                 khi dùng xong, bắt buộc phải gọi để tránh
//                                 rò rỉ bộ nhớ.
//
// Vấn đề: libcurl KHÔNG tự lưu dữ liệu response vào biến cho bạn. Mặc định
// nó chỉ in response ra màn hình console. Để "bắt" được response vào 1
// std::string, bạn phải viết 1 hàm callback (writeCallback bên dưới) và báo
// cho libcurl "mỗi khi có dữ liệu về, hãy gọi hàm này" - đây là kiểu thiết
// kế phổ biến trong các thư viện C cũ, gọi là callback-based API.

// Hàm callback bắt buộc phải đúng chữ ký (signature) này để libcurl gọi
// được (nó gọi qua con trỏ hàm C, không phải qua std::function).
// contents: con trỏ tới dữ liệu vừa nhận được (1 "khúc" dữ liệu, không phải
//           toàn bộ response cùng lúc - libcurl có thể gọi hàm này NHIỀU
//           LẦN cho 1 response dài).
// size * nmemb: tổng số byte của khúc dữ liệu này.
// userp: con trỏ "tự do" bạn truyền vào từ trước (ở đây là std::string* để
//        nối dữ liệu vào).
static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string* buffer = static_cast<std::string*>(userp);
    buffer->append(static_cast<char*>(contents), totalSize);
    return totalSize; // Bắt buộc trả về đúng số byte đã xử lý, nếu trả về
    // khác đi, libcurl hiểu là có lỗi và sẽ huỷ request.
}

LLMResponse OllamaClient::generate(const std::vector<ChatMessage>& messages,
    const LLMConfig& config) {
    // --- Bước 1: build JSON request bằng nlohmann::json ---
    // nlohmann::json cho phép tạo JSON gần giống cú pháp khởi tạo bình
    // thường của C++ (dùng {}), rất khác so với việc phải tự nối string
    // "{\"model\":\"...\"}" thủ công (dễ sai, khó đọc).
    nlohmann::json requestBody;
    requestBody["model"] = config.model;
    requestBody["stream"] = false; // false = đợi nhận đủ toàn bộ câu trả lời
    // 1 lần, thay vì nhận từng chữ một
    // (streaming). Streaming phức tạp hơn
    // (phải đọc dữ liệu tăng dần), nên bản
    // đầu tiên này dùng non-streaming cho
    // dễ, có thể nâng cấp sau nếu cần.

// json::array() tạo ra 1 mảng JSON rỗng []. Sau đó push_back() từng
// tin nhắn vào - giống hệt push_back() của std::vector bạn đã quen.
    nlohmann::json messagesJson = nlohmann::json::array();
    for (const auto& msg : messages) {
        nlohmann::json m;
        m["role"] = msg.role;
        m["content"] = msg.content;
        if (msg.images.has_value()) { // has_value(): kiểm tra optional có
            // dữ liệu bên trong hay không.
            m["images"] = msg.images.value(); // .value(): lấy dữ liệu bên
            // trong optional ra.
        }
        messagesJson.push_back(m);
    }
    requestBody["messages"] = messagesJson;

    // .dump() chuyển object nlohmann::json thành 1 chuỗi text JSON thật sự
    // (kiểu {"model":"...","messages":[...]}) để gửi qua mạng - vì HTTP chỉ
    // truyền được text/byte, không truyền được "object C++" trực tiếp.
    std::string requestStr = requestBody.dump();

    // --- Bước 2: cấu hình libcurl ---
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw LLMClientException("Khong the khoi tao libcurl (curl_easy_init that bai)");
    }

    std::string responseStr; // nơi writeCallback sẽ đổ dữ liệu response vào
    std::string url = config.baseUrl + "/api/chat";

    // curl_slist: 1 danh sách liên kết (linked list) kiểu C, dùng để gửi
    // các dòng HTTP header. Không dùng std::vector<std::string> được vì
    // libcurl là thư viện C, chỉ hiểu con trỏ curl_slist* của riêng nó.
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    // .c_str(): std::string lưu dữ liệu kiểu C++, nhưng hàm C (libcurl) chỉ
    // nhận con trỏ ký tự kiểu C (const char*), nên phải chuyển đổi qua lại
    // bằng .c_str() mỗi khi gọi hàm thư viện C.
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestStr.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L); // timeout 120 giay - LLM
    // co the tra loi lau,
    // dung de timeout qua
    // ngan roi bao loi oan.

// --- Bước 3: đo thời gian, thực sự gửi request ---
    auto startTime = std::chrono::steady_clock::now();
    CURLcode res = curl_easy_perform(curl);
    auto endTime = std::chrono::steady_clock::now();

    // Dọn dẹp NGAY cả khi có lỗi (dùng biến, không return sớm trước khi
    // cleanup) - nếu return sớm mà quên cleanup, đây chính là 1 dạng
    // memory leak / resource leak.
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        // curl_easy_strerror: hàm có sẵn của libcurl, dịch mã lỗi CURLcode
        // (1 con số) thành 1 câu tiếng Anh dễ đọc.
        throw LLMClientException(
            std::string("Goi Ollama that bai: ") + curl_easy_strerror(res));
    }

    // --- Bước 4: parse JSON trả về ---
    nlohmann::json responseJson;
    try {
        responseJson = nlohmann::json::parse(responseStr);
    }
    catch (const nlohmann::json::parse_error& e) {
        // Server trả về gì đó KHÔNG phải JSON hợp lệ (ví dụ HTML báo lỗi
        // 500, hoặc rỗng) - bắt riêng lỗi này để thông báo rõ ràng hơn,
        // thay vì để chương trình crash không rõ lý do.
        throw LLMClientException(
            std::string("Ollama tra ve JSON khong hop le: ") + e.what());
    }

    LLMResponse result;
    // .value("key", defaultValue): lấy giá trị field "key" trong JSON, nếu
    // field đó không tồn tại thì dùng defaultValue thay vì crash chương
    // trình - AN TOÀN hơn nhiều so với responseJson["key"] (cách này sẽ ném
    // exception nếu field không có).
    result.content = responseJson.value("message", nlohmann::json{})
        .value("content", "");
    result.promptTokens = responseJson.value("prompt_eval_count", 0);
    result.completionTokens = responseJson.value("eval_count", 0);

    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime);
    result.latencyMs = durationMs.count();

    return result;
}