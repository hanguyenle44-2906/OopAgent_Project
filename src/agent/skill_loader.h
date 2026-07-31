#pragma once

#include <string>
#include <vector>
#include <optional>

// 1 skill = 1 file .md, gồm ten, danh sach tu khoa (de matching), va noi
// dung day du (se duoc chen vao system prompt).
struct Skill {
    std::string name;
    std::vector<std::string> keywords;
    std::string content;
};

class SkillLoader {
public:
    // Quet thu muc, doc tat ca file .md thanh danh sach Skill.
    void loadSkills(const std::string& path);

    // Chon skill phu hop nhat voi task dua tren so tu khoa trung khop.
    // Tra ve std::nullopt neu khong co skill nao khop (thay vi tra ve
    // "" gay nham lan giua "khong co" va "co nhung rong").
    std::optional<Skill> selectSkill(const std::string& task) const;

    const std::vector<Skill>& getAllSkills() const { return skills_; }

private:
    std::vector<Skill> skills_;

    // Doc 1 file .md, tach phan "keywords" (dong dau, dang dac biet) va
    // phan noi dung con lai.
    Skill parseSkillFile(const std::string& filePath);
};