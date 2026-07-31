#include "web_tool.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>

// Ham callback giong het cai trong ollama_client.cpp - libcurl van can
// callback nay de bat du lieu response vao 1 std::string.
// Danh "static" de gioi han pham vi (scope) chi trong file .cpp nay -
// tranh xung dot ten voi ham writeCallback trung ten ben ollama_client.cpp
// khi linker ghep 2 file lai (khong co "static" se bao loi "da dinh nghia
// 2 lan").
static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string* buffer = static_cast<std::string*>(userp);
    buffer->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string WebTool::getName() const {
    return "web_search";
}

std::string WebTool::getDescription() const {
    return "Tim kiem thong tin tren web (qua DuckDuckGo). "
        "Tham so: {\"query\": \"cau hoi can tim\"}";
}

std::string WebTool::execute(const std::string& argsJson) {
    try {
        nlohmann::json args = nlohmann::json::parse(argsJson);
        std::string query = args.value("query", "");
        if (query.empty()) {
            return "Loi: thieu tham so query";
        }

        CURL* curl = curl_easy_init();
        if (!curl) {
            return "Loi: khong the khoi tao libcurl";
        }

        // curl_easy_escape: ma hoa lai chuoi query cho an toan de nhet
        // vao URL (vi du dau cach -> %20). Bat buoc phai lam buoc nay,
        // neu khong URL se sai/gay loi khi query co dau cach hoac tieng
        // Viet co dau.
        char* encodedQuery = curl_easy_escape(curl, query.c_str(), static_cast<int>(query.size()));
        std::string url = "https://api.duckduckgo.com/?q=" + std::string(encodedQuery) +
            "&format=json&no_html=1";
        curl_free(encodedQuery); // curl_easy_escape tu cap phat bo nho,
        // phai tu tay giai phong bang curl_free
        // (khong dung "delete" hay "free" thong
        // thuong, vi bo nho nay do libcurl quan
        // ly noi bo).

        std::string responseStr;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
        // CURLOPT_HTTPGET: bao ro day la request GET (mac dinh cua
        // libcurl da la GET, nhung set ro cho de doc, tranh nham voi POST
        // neu sau nay co ai sua code va vo tinh de sot option POST cu).
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            return std::string("Loi: goi web search that bai - ") + curl_easy_strerror(res);
        }

        nlohmann::json responseJson = nlohmann::json::parse(responseStr);

        // DuckDuckGo Instant Answer tra ve nhieu field, quan trong nhat la
        // "AbstractText" (doan tom tat) va "RelatedTopics" (cac chu de
        // lien quan). Nhieu truy van se ra rong (khong co Instant Answer
        // phu hop) - day la han che thuc su cua API mien phi nay.
        std::string abstractText = responseJson.value("AbstractText", "");
        if (!abstractText.empty()) {
            std::string source = responseJson.value("AbstractSource", "");
            return abstractText + (source.empty() ? "" : " (Nguon: " + source + ")");
        }

        // Neu khong co AbstractText, thu lay chu de lien quan dau tien.
        if (responseJson.contains("RelatedTopics") &&
            !responseJson["RelatedTopics"].empty()) {
            auto& first = responseJson["RelatedTopics"][0];
            if (first.contains("Text")) {
                return first.value("Text", "");
            }
        }

        return "Khong tim thay ket qua Instant Answer cho truy van nay.";
    }
    catch (const std::exception& e) {
        return std::string("Loi: ") + e.what();
    }
}