#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <iostream>

class TrajectoryLogger {
private:
    nlohmann::json logData;

public:
    TrajectoryLogger() {
        logData["steps"] = nlohmann::json::array();
    }

    // Lắng nghe và ghi chép từng bước (Observer Pattern)
    void logStep(int stepNum, const std::string& thought, const std::string& action, const std::string& observation) {
        nlohmann::json stepItem;
        stepItem["step"] = stepNum;
        stepItem["thought"] = thought;
        stepItem["action"] = action;
        stepItem["observation"] = observation;
        
        logData["steps"].push_back(stepItem);
    }

    // Xuất ra file JSON
    void saveToFile(const std::string& filename = "trajectory_log.json") {
        std::ofstream outFile(filename);
        if (outFile.is_open()) {
            outFile << logData.dump(4); // indent 4 cách cho đẹp
            outFile.close();
            std::cout << "[TrajectoryLogger] Log saved to " << filename << std::endl;
        } else {
            std::cerr << "[TrajectoryLogger] Error: Could not save log file." << std::endl;
        }
    }
};