#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

namespace jar {

class Automation;

class AutomationRegistry {
public:
    void
    add(std::shared_ptr<Automation> automation);

    bool
    has(const std::string& intent) const;

    std::shared_ptr<Automation>
    get(const std::string& intent);

private:
    mutable std::mutex _guard;
    std::unordered_map<std::string, std::shared_ptr<Automation>> _registry;
};

} // namespace jar