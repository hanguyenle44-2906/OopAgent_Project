#pragma once

#include "tool.h"
#include <string>

// Forward declaration: bao cho compiler "co 1 struct ten sqlite3 ton tai o
// dau do", ma KHONG can #include <sqlite3.h> ngay trong file .h nay.
// Loi ich: file nao khac #include memory_tool.h se khong bi "keo theo"
// toan bo sqlite3.h khong can thiet - giam thoi gian bien dich. Chi file
// .cpp (noi thuc su dung sqlite3) moi can #include <sqlite3.h> that su.
struct sqlite3;

// MemoryTool: luu va tim lai thong tin qua SQLite.
// argsJson mong doi dang:
//   {"action": "save", "key": "ten_bien", "content": "gia tri can nho"}
//   {"action": "search", "query": "tu khoa can tim"}
class MemoryTool : public Tool {
public:
    MemoryTool();
    ~MemoryTool() override; // can dinh nghia rieng (khong dung = default)
    // vi phai tu dong sqlite3_close() trong .cpp

    std::string getName() const override;
    std::string getDescription() const override;
    std::string execute(const std::string& argsJson) override;

private:
    std::string save(const std::string& key, const std::string& content);
    std::string search(const std::string& query);

    sqlite3* db_ = nullptr;
};