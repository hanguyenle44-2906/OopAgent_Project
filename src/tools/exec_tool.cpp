#include "exec_tool.h"
#include <nlohmann/json.hpp>
#include <array>
#include <memory>
#include <stdexcept>

std::string ExecTool::getName() const {
    return "exec";
}

std::string ExecTool::getDescription() const {
    return "Chay 1 lenh shell va tra ve output. Tham so: {\"command\": \"echo hello\"}";
}

std::string ExecTool::execute(const std::string& argsJson) {
    try {
        nlohmann::json args = nlohmann::json::parse(argsJson);
        std::string command = args.value("command", "");
        if (command.empty()) {
            return "Loi: thieu tham so command";
        }

        // #ifdef _WIN32: tren Windows, ham ten la _popen/_pclose (co gach
        // duoi). Tren Linux/macOS (nhu may cua dong doi ban), ham ten la
        // popen/pclose (khong gach duoi). 2 ham lam VIEC GIONG HET NHAU,
        // chi khac ten goi - day la 1 trong nhung cho hiem hoi C++
        // "khong hoan toan giong nhau" giua cac he dieu hanh, phai tach
        // rieng bang macro nhu the nay.
#ifdef _WIN32
        FILE* pipe = _popen(command.c_str(), "r");
#else
        FILE* pipe = popen(command.c_str(), "r");
#endif
        if (!pipe) {
            return "Loi: khong the chay lenh (popen that bai)";
        }

        // Doc output cua lenh, tung khuc 256 byte mot, cho toi khi het
        // (fgets tra ve nullptr nghia la da doc het / gap loi).
        std::string result;
        std::array<char, 256> buffer;
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }

#ifdef _WIN32
        _pclose(pipe);
#else
        pclose(pipe);
#endif

        if (result.empty()) {
            return "(lenh chay xong, khong co output)";
        }
        return result;
    }
    catch (const std::exception& e) {
        return std::string("Loi: ") + e.what();
    }
}