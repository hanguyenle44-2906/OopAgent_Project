#include "skill_loader.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

// Quy uoc dinh dang file .md:
//   dong dau tien: "keywords: tu1, tu2, tu3"
//   cac dong con lai: noi dung skill (se duoc chen vao system prompt)
Skill SkillLoader::parseSkillFile(const std::string& filePath) {
    Skill skill;
    skill.name = fs::path(filePath).stem().string(); // stem(): lay ten
    // file KHONG kem
    // duoi ".md"

    std::ifstream file(filePath);
    std::string firstLine;
    std::getline(file, firstLine);

    const std::string prefix = "keywords:";
    if (firstLine.size() >= prefix.size() &&
        firstLine.compare(0, prefix.size(), prefix) == 0) {
        std::string keywordsPart = firstLine.substr(prefix.size());
        std::stringstream ss(keywordsPart);
        std::string kw;
        while (std::getline(ss, kw, ',')) {
            // Xoa khoang trang thua o dau/cuoi tu khoa.
            size_t start = kw.find_first_not_of(" \t");
            size_t end = kw.find_last_not_of(" \t");
            if (start != std::string::npos) {
                skill.keywords.push_back(kw.substr(start, end - start + 1));
            }
        }
    }
    else {
        // Neu dong dau khong phai "keywords:", coi nhu ca file la noi
        // dung, khong co keyword rieng (van doc duoc, chi khong match
        // theo tu khoa).
        file.seekg(0); // seekg(0): dua con tro doc file ve lai dau file,
        // vi getline() o tren da "an" mat dong dau.
    }

    std::stringstream contentBuffer;
    contentBuffer << file.rdbuf();
    skill.content = contentBuffer.str();
    return skill;
}

void SkillLoader::loadSkills(const std::string& path) {
    skills_.clear();
    if (!fs::exists(path) || !fs::is_directory(path)) {
        return;
    }

    // fs::directory_iterator: duyet qua TUNG FILE/THU MUC con truc tiep
    // ben trong "path" (khong duyet sau vao thu muc con - neu can duyet
    // ca thu muc con thi dung recursive_directory_iterator thay the).
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".md") {
            skills_.push_back(parseSkillFile(entry.path().string()));
        }
    }
}

std::optional<Skill> SkillLoader::selectSkill(const std::string& task) const {
    // Chuyen task ve chu thuong de so khop khong phan biet hoa/thuong.
    std::string taskLower = task;
    std::transform(taskLower.begin(), taskLower.end(), taskLower.begin(),
        [](unsigned char c) { return std::tolower(c); });

    const Skill* bestMatch = nullptr;
    int bestScore = 0;

    for (const auto& skill : skills_) {
        int score = 0;
        for (const auto& keyword : skill.keywords) {
            std::string kwLower = keyword;
            std::transform(kwLower.begin(), kwLower.end(), kwLower.begin(),
                [](unsigned char c) { return std::tolower(c); });
            if (taskLower.find(kwLower) != std::string::npos) {
                score++;
            }
        }
        if (score > bestScore) {
            bestScore = score;
            bestMatch = &skill;
        }
    }

    if (bestMatch == nullptr) {
        return std::nullopt;
    }
    return *bestMatch;
}