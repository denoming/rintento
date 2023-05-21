#include "intent/ActionPerformer.hpp"

#include "intent/Action.hpp"
#include "intent/ActionRegistry.hpp"

#include <jarvis/Logger.hpp>

#include <ranges>

namespace jar {

namespace {

Intents::const_iterator
mostConfidentIntent(const Intents& intents)
{
    return std::max_element(
        intents.cbegin(), intents.cend(), [](const Intent& n1, const Intent& n2) {
            return (n1.confidence < n2.confidence);
        });
}

} // namespace

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

    auto filteredUtterances = utterances | std::views::filter([](const Utterance& u) {
                                  return u.final && !u.intents.empty();
                              });

    for (auto&& utterance : filteredUtterances) {
        if (const auto intentIt = mostConfidentIntent(utterance.intents);
            _registry.has(intentIt->name)) {
            LOGI("Add action for <{}> intent", intentIt->name);
            auto action = _registry.get(intentIt->name, std::move(utterance.entities));
            action->onDone(sigc::mem_fun(*this, &ActionPerformer::onActionDone));
            _pendingActions.push(action);
        }
    }

    if (!_pendingActions.empty()) {
        auto& action = _pendingActions.front();
        LOGI("Perform action for <{}> intent", action->intent());
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