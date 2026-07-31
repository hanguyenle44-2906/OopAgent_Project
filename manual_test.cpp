#include "src/agent/skill_loader.h"
#include <iostream>
#include <filesystem>

int main() {
    SkillLoader loader;
    loader.loadSkills("skills");

    std::cout << "So skill da load: " << loader.getAllSkills().size() << "\n";

    auto skill = loader.selectSkill("Toi can tim kiem thong tin ve gia vang hom nay");
    if (skill.has_value()) {
        std::cout << "Skill phu hop: " << skill->name << "\n";
    }
    else {
        std::cout << "Khong tim thay skill phu hop\n";
    }

    return 0;
}