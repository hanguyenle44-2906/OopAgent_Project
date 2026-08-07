#ifndef TOOL_SECURITY_POLICY_H
#define TOOL_SECURITY_POLICY_H

#include <string>
#include <set>
#include <stdexcept>
#include <sstream>

class SecurityPolicyViolation : public std::runtime_error {
public:
    explicit SecurityPolicyViolation(const std::string& msg) 
        : std::runtime_error("Security Policy Violation: " + msg) {}
};

class ToolSecurityPolicy {
private:
    std::set<std::string> whitelist_;
    std::set<std::string> blacklist_;

public:
    ToolSecurityPolicy() {
        whitelist_ = {"ls", "cat", "grep", "ping", "echo", "pwd", "head", "tail"};
        blacklist_ = {"rm", "sudo", "chown", "chmod", "mkfs", "dd", ">", ">>"};
    }

    inline void validate_command(const std::string& full_command) const {
        for (const auto& blocked : blacklist_) {
            if (full_command.find(blocked) != std::string::npos) {
                throw SecurityPolicyViolation("Forbidden keyword detected: '" + blocked + "'");
            }
        }

        std::stringstream ss(full_command);
        std::string base_cmd;
        ss >> base_cmd;

        size_t last_slash = base_cmd.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            base_cmd = base_cmd.substr(last_slash + 1);
        }

        if (whitelist_.find(base_cmd) == whitelist_.end()) {
            throw SecurityPolicyViolation("Command '" + base_cmd + "' is not whitelisted!");
        }
    }
};

#endif