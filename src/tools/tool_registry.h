#pragma once

#include "tool.h"
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

class ToolPolicy {
public:
    void setAllowList(std::set<std::string> names);
    void setDenyList(std::set<std::string> names);
    bool isAllowed(const std::string& toolName) const;

private:
    std::set<std::string> allowList_;
    std::set<std::string> denyList_;
};

using ToolFactory = std::function<std::unique_ptr<Tool>()>;

class ToolRegistry {
public:
    void registerTool(const std::string& name, ToolFactory factory);
    std::unique_ptr<Tool> createTool(const std::string& name) const;
    std::vector<std::pair<std::string, std::string>> listTools() const;
    ToolPolicy& policy();

private:
    std::map<std::string, ToolFactory> factories_;
    ToolPolicy policy_;
};