#pragma once

#include "intent/Types.hpp"
#include "intent/Intent.hpp"

#include <memory>
#include <queue>

namespace jar {

class IntentRegistry;

class IntentPerformer : public std::enable_shared_from_this<IntentPerformer> {
public:
    using Ptr = std::shared_ptr<IntentPerformer>;

    static std::shared_ptr<IntentPerformer>
    create(IntentRegistry& registry);

    void
    perform(UtteranceSpecs utterances);

private:
    IntentPerformer(IntentRegistry& registry);

    void
    onIntentComplete(std::error_code);

private:
    IntentRegistry& _registry;

    std::queue<Intent::Ptr> _pendingIntents;
};

} // namespace jar