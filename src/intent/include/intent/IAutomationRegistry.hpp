#pragma once

#include <memory>
#include <string>

namespace jar {

class Automation;

class IAutomationRegistry {
public:
    using Ptr = std::shared_ptr<IAutomationRegistry>;

    virtual ~IAutomationRegistry() = default;

    virtual void
    add(std::shared_ptr<Automation> automation)
        = 0;

    [[nodiscard]] virtual bool
    has(const std::string& intent) const
        = 0;

    virtual std::shared_ptr<Automation>
    get(const std::string& intent) = 0;
};

} // namespace jar