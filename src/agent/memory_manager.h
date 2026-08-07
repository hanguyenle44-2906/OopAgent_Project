#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <sqlite3.h>
#include <string>
#include <optional>
#include <memory>

class MemoryManager {
private:
    struct Sqlite3Closer {
        void operator()(sqlite3* db) const { if (db) sqlite3_close(db); }
    };
    std::unique_ptr<sqlite3, Sqlite3Closer> db_ptr_;

public:
    explicit MemoryManager(const std::string& db_path) {
        sqlite3* raw_db = nullptr;
        sqlite3_open(db_path.c_str(), &raw_db);
        db_ptr_.reset(raw_db);
        const char* sql = "CREATE TABLE IF NOT EXISTS history(key TEXT, context TEXT);";
        sqlite3_exec(db_ptr_.get(), sql, nullptr, nullptr, nullptr);
    }

    std::optional<std::string> query_memory(const std::string& key) const {
        if (!db_ptr_) return std::nullopt;
        sqlite3_stmt* stmt;
        std::string q = "SELECT context FROM history WHERE key = ? ORDER BY rowid DESC LIMIT 1;";
        if (sqlite3_prepare_v2(db_ptr_.get(), q.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
        
        std::optional<std::string> result = std::nullopt;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        }
        sqlite3_finalize(stmt);
        return result;
    }

    // Bổ sung hàm save_memory để fix lỗi không tìm thấy hàm
    void save_memory(const std::string& key, const std::string& context) {
        if (!db_ptr_) return;
        std::string sql = "INSERT INTO history(key, context) VALUES(?, ?);";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db_ptr_.get(), sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, context.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
};
#endif