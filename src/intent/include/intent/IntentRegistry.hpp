#pragma once

#include "intent/IIntentRegistry.hpp"

#include <unordered_map>

namespace jar {

class IntentRegistry final : public IIntentRegistry {
public:
    IntentRegistry() = default;

    bool
    has(const std::string& name) const final;

    std::shared_ptr<Intent>
    get(const std::string& name) final;

    void
    add(std::shared_ptr<Intent> intent) final;

    void
    remove(const std::string& name) final;

private:
    std::unordered_map<std::string, std::shared_ptr<Intent>> _intents;
};

} // namespace jar