#pragma once

#include "intent/Types.hpp"

#include <memory>
#include <queue>

namespace jar {

class Intent;
class IntentRegistry;

class IntentPerformer : public std::enable_shared_from_this<IntentPerformer> {
public:
    static std::shared_ptr<IntentPerformer>
    create(IntentRegistry& registry);

    void
    perform(UtteranceSpecs utterances);

private:
    IntentPerformer(IntentRegistry& registry);

    void onIntentComplete(std::error_code);

private:
    IntentRegistry& _registry;

    std::queue<std::shared_ptr<Intent>> _pendingIntents;
};

} // namespace jar