#pragma once

#include <memory>
#include <string>

namespace jar {

class Automation;

class IAutomationRegistry {
public:
    virtual ~IAutomationRegistry() = default;

    virtual void
    add(std::shared_ptr<Automation> automation)
        = 0;

    virtual bool
    has(const std::string& intent) const
        = 0;

    virtual std::shared_ptr<Automation>
    get(const std::string& intent) = 0;
};

} // namespace jar