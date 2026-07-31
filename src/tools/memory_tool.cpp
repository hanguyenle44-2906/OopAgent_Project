#include "memory_tool.h"

#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include <sstream>

MemoryTool::MemoryTool() {
    // sqlite3_open: mo file "memory.db" (tu tao moi neu chua ton tai).
    // Tham so dau: duong dan file. Tham so 2: dia chi con tro db_ - ham
    // nay se GAN GIA TRI vao db_ thong qua con tro toi con tro (sqlite3**)
    // - day la ly do phai truyen &db_ (lay dia chi cua db_) thay vi db_.
    int rc = sqlite3_open("memory.db", &db_);
    if (rc != SQLITE_OK) {
        db_ = nullptr;
        return; // Khong throw exception trong constructor tool o day de
        // don gian - execute() se tu kiem tra db_ == nullptr.
    }

    // Tao bang "memory" neu chua ton tai. Cu phap SQL:
    //   TEXT PRIMARY KEY: cot "key" la kieu chuoi, va la "khoa chinh"
    //   (khong duoc trung nhau giua cac dong, dung de tra cuu nhanh).
    const char* createTableSql =
        "CREATE TABLE IF NOT EXISTS memory ("
        "  key TEXT PRIMARY KEY,"
        "  content TEXT NOT NULL"
        ");";

    // sqlite3_exec: chay 1 cau SQL "tinh" (khong co du lieu nguoi dung
    // chen vao dong nay, nen dung truc tiep exec la an toan, khong can
    // prepared statement).
    char* errMsg = nullptr;
    rc = sqlite3_exec(db_, createTableSql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg); // errMsg do sqlite3 tu cap phat, phai tu
        // giai phong bang sqlite3_free (khong dung
        // delete/free thong thuong).
    }
}

MemoryTool::~MemoryTool() {
    if (db_) {
        sqlite3_close(db_);
    }
}

std::string MemoryTool::getName() const {
    return "memory";
}

std::string MemoryTool::getDescription() const {
    return "Luu va tim lai thong tin. Tham so: "
        "{\"action\":\"save\",\"key\":\"ten\",\"content\":\"gia tri\"} hoac "
        "{\"action\":\"search\",\"query\":\"tu khoa\"}";
}

std::string MemoryTool::execute(const std::string& argsJson) {
    if (!db_) {
        return "Loi: khong the mo database memory.db";
    }
    try {
        nlohmann::json args = nlohmann::json::parse(argsJson);
        std::string action = args.value("action", "");

        if (action == "save") {
            return save(args.value("key", ""), args.value("content", ""));
        }
        else if (action == "search") {
            return search(args.value("query", ""));
        }
        return "Loi: action phai la 'save' hoac 'search'";
    }
    catch (const std::exception& e) {
        return std::string("Loi: ") + e.what();
    }
}

std::string MemoryTool::save(const std::string& key, const std::string& content) {
    // "INSERT OR REPLACE": neu key da ton tai, GHI DE gia tri moi thay vi
    // bao loi trung khoa chinh - phu hop voi kieu du lieu "memory" (nho
    // lai gia tri moi nhat cho 1 cai ten).
    // Dau "?1", "?2": cho trong (placeholder) se duoc "bind" gia tri that
    // vao ben duoi - day chinh la prepared statement, AN TOAN hon nhieu
    // so voi viec noi chuoi SQL truc tiep voi key/content (nguoi dung co
    // the go noi dung chua ky tu dac biet nhu dau nhay don, gay loi SQL
    // hoac te hon la SQL injection neu noi chuoi tay).
    const char* sql = "INSERT OR REPLACE INTO memory (key, content) VALUES (?1, ?2);";

    sqlite3_stmt* stmt = nullptr;
    // sqlite3_prepare_v2: "bien dich truoc" cau SQL, chua chay that.
    // Tham so -1: bao sqlite3 tu tinh do dai chuoi sql (vi no ket thuc
    // bang ky tu null '\0' nhu chuoi C thong thuong).
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return "Loi: khong the chuan bi cau lenh SQL";
    }

    // Gan gia tri that vao cho "?1" va "?2". Tham so cuoi (-1, SQLITE_TRANSIENT)
    // bao sqlite3 TU COPY lai noi dung chuoi (an toan, vi std::string co
    // the bi huy sau khi ham nay return, sqlite3 khong nen giu con tro
    // truc tiep toi du lieu cua std::string).
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_TRANSIENT);

    // sqlite3_step: THUC SU chay cau lenh. Voi INSERT, ket qua mong doi
    // la SQLITE_DONE (nghia la chay xong, khong co dong du lieu tra ve -
    // khac voi SELECT se tra ve SQLITE_ROW cho tung dong ket qua).
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt); // Giai phong statement, LUON goi du thanh
    // cong hay that bai - giong nguyen tac
    // curl_easy_cleanup() truoc day.

    if (rc != SQLITE_DONE) {
        return "Loi: luu du lieu that bai";
    }
    return "Da luu: " + key;
}

std::string MemoryTool::search(const std::string& query) {
    // "LIKE '%...%'": tim cac dong ma cot "content" CHUA chuoi query o
    // bat ky vi tri nao (dau % la ky tu dai dien, nghia la "bat ky gi").
    const char* sql = "SELECT key, content FROM memory WHERE content LIKE ?1;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return "Loi: khong the chuan bi cau lenh SQL";
    }

    std::string likePattern = "%" + query + "%";
    sqlite3_bind_text(stmt, 1, likePattern.c_str(), -1, SQLITE_TRANSIENT);

    std::stringstream result;
    int foundCount = 0;
    // sqlite3_step tra ve SQLITE_ROW cho MOI dong ket qua tim duoc - vong
    // lap while nay chay lien tuc cho toi khi het dong (luc do tra ve
    // SQLITE_DONE, thoat vong lap).
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        // sqlite3_column_text: doc gia tri cot theo VI TRI (0 = cot dau
        // tien trong SELECT, tuc "key"; 1 = cot thu 2, tuc "content").
        // Can ep kieu (const char*) vi ham nay tra ve kieu rieng cua
        // sqlite3 (unsigned char*), khong phai const char* tieu chuan.
        const char* key = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        result << key << ": " << content << "\n";
        foundCount++;
    }
    sqlite3_finalize(stmt);

    if (foundCount == 0) {
        return "Khong tim thay ket qua nao cho: " + query;
    }
    return result.str();
}