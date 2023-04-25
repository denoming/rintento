#pragma once

#include "intent/WitTypes.hpp"

#include <memory>
#include <string>

namespace jar {

class Action;

class IActionRegistry {
public:
    [[nodiscard]] virtual bool
    has(const std::string& name) const
        = 0;

    [[nodiscard]] virtual std::shared_ptr<Action>
    get(const std::string& name, Entities entities) = 0;

    virtual void
    add(std::shared_ptr<Action> intent)
        = 0;

    virtual void
    remove(const std::string& name)
        = 0;
};

} // namespace jar