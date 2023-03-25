#pragma once

#include "intent/Intent.hpp"

#include <functional>

namespace jar {

class IIntentRegistry {
public:
    [[nodiscard]] virtual bool
    has(const std::string& name) const
        = 0;

    [[nodiscard]] virtual Intent::Ptr
    get(const std::string& name)
        = 0;

    virtual void
    add(Intent::Ptr intent)
        = 0;

    virtual void
    remove(const std::string& name)
        = 0;
};

} // namespace jar