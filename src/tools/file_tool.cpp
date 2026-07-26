#include "file_tool.h"

#include <nlohmann/json.hpp>
#include <filesystem> // std::filesystem - tinh nang C++17 bat buoc
#include <fstream>
#include <sstream>

// namespace alias: dat ten ngan "fs" thay vi go "std::filesystem" nhieu
// lan cho met - chi la 1 cach "dat biet danh", khong tao ra thu gi moi.
namespace fs = std::filesystem;

std::string FileTool::getName() const {
    return "file";
}

std::string FileTool::getDescription() const {
    return "Doc/ghi file. Tham so: "
        "{\"action\":\"read\",\"path\":\"a.txt\"} hoac "
        "{\"action\":\"write\",\"path\":\"a.txt\",\"content\":\"...\"}";
}

std::string FileTool::execute(const std::string& argsJson) {
    try {
        nlohmann::json args = nlohmann::json::parse(argsJson);
        std::string action = args.value("action", "");
        std::string path = args.value("path", "");

        if (action == "read") {
            return readFile(path);
        }
        else if (action == "write") {
            std::string content = args.value("content", "");
            return writeFile(path, content);
        }
        else {
            return "Loi: action phai la 'read' hoac 'write'";
        }
    }
    catch (const std::exception& e) {
        return std::string("Loi: ") + e.what();
    }
}

std::string FileTool::readFile(const std::string& path) {
    // fs::exists: kiem tra file/thu muc co ton tai khong, TRUOC KHI thu
    // mo file - giup bao loi ro rang ("khong tim thay file") thay vi de
    // ifstream mo that bai roi khong biet ly do vi sao.
    if (!fs::exists(path)) {
        return "Loi: khong tim thay file " + path;
    }
    if (fs::is_directory(path)) {
        return "Loi: " + path + " la mot thu muc, khong phai file";
    }

    // std::ifstream mo file de doc (input file stream).
    std::ifstream file(path);
    if (!file.is_open()) {
        return "Loi: khong the mo file " + path;
    }

    // std::stringstream + rdbuf(): cach ngan gon de doc TOAN BO noi dung
    // file vao 1 chuoi, thay vi tu viet vong lap doc tung dong.
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string FileTool::writeFile(const std::string& path, const std::string& content) {
    // std::ofstream mo file de ghi (output file stream). Neu file chua
    // ton tai, no se TU TAO MOI. Neu da ton tai, mac dinh se GHI DE (xoa
    // noi dung cu) - can luu y neu sau nay muon ho tro "append" (ghi noi
    // tiep) thi phai truyen them co std::ios::app.
    std::ofstream file(path);
    if (!file.is_open()) {
        return "Loi: khong the tao/mo file " + path + " de ghi";
    }
    file << content;
    file.close();
    return "Da ghi " + std::to_string(content.size()) + " ky tu vao " + path;
}