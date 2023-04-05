#pragma once

#include <memory>
#include <string>

namespace jar {

class Intent;

class IIntentRegistry {
public:
    [[nodiscard]] virtual bool
    has(const std::string& name) const
        = 0;

    [[nodiscard]] virtual std::shared_ptr<Intent>
    get(const std::string& name) = 0;

    virtual void
    add(std::shared_ptr<Intent> intent)
        = 0;

    virtual void
    remove(const std::string& name)
        = 0;
};

} // namespace jar