#pragma once

#include "intent/IIntentRegistry.hpp"

#include <unordered_map>

namespace jar {

class IntentRegistry final : public IIntentRegistry {
public:
    using Ptr = std::unique_ptr<IntentRegistry>;

    IntentRegistry() = default;

    bool
    has(const std::string& name) const final;

    Intent::Ptr
    get(const std::string& name) final;

    void
    add(Intent::Ptr intent) final;

    void
    remove(const std::string& name) final;

private:
    std::unordered_map<std::string, Intent::Ptr> _intents;
};

} // namespace jar