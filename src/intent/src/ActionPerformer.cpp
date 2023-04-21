#include "intent/ActionPerformer.hpp"

#include "intent/Action.hpp"
#include "intent/ActionRegistry.hpp"
#include "jarvis/Logger.hpp"

#include <ranges>

namespace jar {

std::shared_ptr<ActionPerformer>
ActionPerformer::create(ActionRegistry& registry)
{
    return std::shared_ptr<ActionPerformer>(new ActionPerformer{registry});
}

ActionPerformer::ActionPerformer(ActionRegistry& registry)
    : _registry{registry}
{
}

void
ActionPerformer::perform(Utterances utterances)
{
    LOGI("The <{}> utterances are available", utterances.size());

    auto filtered = utterances | std::views::filter([](const auto& u) { return u.final; });
    for (auto&& utterance : filtered) {
        for (auto&& intent : utterance.intents) {
            if (_registry.has(intent.name)) {
                LOGI("Add <{}> action to the pending queue", intent.name);
                auto action = _registry.get(intent.name);
                action->onDone(sigc::mem_fun(*this, &ActionPerformer::onActionDone));
                _pendingActions.push(action);
                break;
            }
        }
    }

    if (!_pendingActions.empty()) {
        auto& action = _pendingActions.front();
        LOGI("Perform <{}> action", action->intent());
        action->perform();
    } else {
        LOGE("No pending actions");
    }
}

void
ActionPerformer::onActionDone(std::error_code error)
{
    if (error) {
        LOGE("Action performing has failed: {}", error.message());
    } else {
        LOGI("Action performing has done");
    }
}

} // namespace jar