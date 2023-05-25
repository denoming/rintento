#pragma once

#include "intent/IActionRegistry.hpp"

#include <unordered_map>

namespace jar {

class ActionRegistry final : public IActionRegistry {
public:
    ActionRegistry() = default;

    bool
    has(const std::string& intent) const final;

    std::shared_ptr<Action>
    get(const std::string& intent, wit::Entities entities) final;

    void
    add(std::shared_ptr<Action> action) final;

    void
    remove(const std::string& intent) final;

private:
    std::unordered_map<std::string, std::shared_ptr<Action>> _actions;
};

} // namespace jar