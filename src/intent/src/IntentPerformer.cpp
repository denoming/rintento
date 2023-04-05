#include "intent/IntentPerformer.hpp"

#include "intent/Intent.hpp"
#include "intent/IntentRegistry.hpp"
#include "jarvis/Logger.hpp"

#include <ranges>

namespace jar {

std::shared_ptr<IntentPerformer>
IntentPerformer::create(IntentRegistry& registry)
{
    return std::shared_ptr<IntentPerformer>(new IntentPerformer{registry});
}

IntentPerformer::IntentPerformer(IntentRegistry& registry)
    : _registry{registry}
{
}

void
IntentPerformer::perform(UtteranceSpecs utterances)
{
    LOGI("The <{}> utterances are available", utterances.size());

    auto filtered = utterances | std::views::filter([](const auto& u) { return u.final; });
    for (auto&& utterance : filtered) {
        for (auto&& intent : utterance.intents) {
            if (_registry.has(intent.name)) {
                LOGI("Add <{}> intent to the pending queue", intent.name);
                _pendingIntents.push(_registry.get(intent.name));
                break;
            }
        }
    }

    if (!_pendingIntents.empty()) {
        auto& intent = _pendingIntents.front();
        LOGI("Perform <{}> intent", intent->name());
        intent->perform(std::bind_front(&IntentPerformer::onIntentComplete, this));
    } else {
        LOGE("No pending intents");
    }
}

void
IntentPerformer::onIntentComplete(std::error_code error)
{
    if (error) {
        LOGE("Failed to perform intent: {}", error.message());
    } else {
        LOGI("Performing of intent was successfully");
    }
}

} // namespace jar