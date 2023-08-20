#pragma once

#include "intent/IAutomationRegistry.hpp"

#include <mutex>
#include <unordered_map>

namespace jar {

class AutomationRegistry final : public IAutomationRegistry {
public:
    void
    add(std::shared_ptr<Automation> automation) final;

    bool
    has(const std::string& intent) const final;

    std::shared_ptr<Automation>
    get(const std::string& intent) final;

private:
    mutable std::mutex _guard;
    std::unordered_map<std::string, std::shared_ptr<Automation>> _registry;
};

} // namespace jar