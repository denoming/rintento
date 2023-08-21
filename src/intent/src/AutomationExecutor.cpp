#include "intent/AutomationExecutor.hpp"

#include "intent/Automation.hpp"
#include "intent/AutomationRegistry.hpp"

#include <jarvisto/Logger.hpp>

#include <ranges>

namespace jar {

namespace {

wit::Intents::const_iterator
mostConfidentIntent(const wit::Intents& intents)
{
    return std::max_element(
        intents.cbegin(), intents.cend(), [](const wit::Intent& n1, const wit::Intent& n2) {
            return (n1.confidence < n2.confidence);
        });
}

} // namespace

AutomationExecutor::AutomationExecutor(AutomationRegistry& registry)
    : _registry{registry}
{
}

void
AutomationExecutor::execute(const wit::Utterances& utterances)
{
    auto filteredUtterances = utterances | std::views::filter([](const wit::Utterance& u) {
                                  return u.final && !u.intents.empty();
                              });

    for (auto&& utterance : filteredUtterances) {
        const auto intentIt = mostConfidentIntent(utterance.intents);
        BOOST_ASSERT(intentIt != std::cend(utterance.intents));
        if (_registry.has(intentIt->name)) {
            execute(intentIt->name);
        } else {
            LOGE("Unable to find automation for <{}> intent", intentIt->name);
        }
    }
}

void
AutomationExecutor::execute(const std::string& intent)
{
    Automation::Ptr automation;

    if (automation = _registry.get(intent); not automation) {
        LOGE("Unable to find automation for <{}> intent", intent);
        return;
    }

    BOOST_ASSERT(automation);
    const auto id = automation->id();
    _runningList.insert({id, automation});

    const auto alias = automation->alias();
    automation->onDone([id, alias, weakSelf = weak_from_this()](std::error_code ec) {
        if (auto self = weakSelf.lock()) {
            self->onAutomationDone(id, alias, ec);
        }
    });

    LOGI("Execute <{} ({})> automation", alias, id);
    automation->execute();
}

void
AutomationExecutor::onAutomationDone(const std::string& id,
                                     const std::string& alias,
                                     std::error_code ec)
{
    LOGI("The <{} ({})> automation is done: result<{}>", id, alias, ec.message());

    if (const auto count = _runningList.erase(id); count > 0) {
        LOGD("Remove <{} ({})> automation from running list", id, alias);
    }
}

} // namespace jar