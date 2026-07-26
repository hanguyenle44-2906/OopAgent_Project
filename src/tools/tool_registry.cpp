#include "tool_registry.h"

// ===== ToolPolicy =====

void ToolPolicy::setAllowList(std::set<std::string> names) {
    allowList_ = std::move(names);
}

void ToolPolicy::setDenyList(std::set<std::string> names) {
    denyList_ = std::move(names);
}

bool ToolPolicy::isAllowed(const std::string& toolName) const {
    if (denyList_.count(toolName) > 0) {
        return false;
    }
    if (!allowList_.empty() && allowList_.count(toolName) == 0) {
        return false;
    }
    return true;
}

// ===== ToolRegistry =====

void ToolRegistry::registerTool(const std::string& name, ToolFactory factory) {
    factories_[name] = std::move(factory);
}

std::unique_ptr<Tool> ToolRegistry::createTool(const std::string& name) const {
    if (!policy_.isAllowed(name)) {
        return nullptr;
    }
    auto it = factories_.find(name);
    if (it == factories_.end()) {
        return nullptr;
    }
    return it->second();
}

std::vector<std::pair<std::string, std::string>> ToolRegistry::listTools() const {
    std::vector<std::pair<std::string, std::string>> result;
    for (const auto& [name, factory] : factories_) {
        if (!policy_.isAllowed(name)) {
            continue;
        }
        auto tempTool = factory();
        result.emplace_back(tempTool->getName(), tempTool->getDescription());
    }
    return result;
}

ToolPolicy& ToolRegistry::policy() {
    return policy_;
}