#include "intent/AutomationPerformer.hpp"

#include "intent/Automation.hpp"
#include "intent/AutomationRegistry.hpp"

#include <jarvisto/Logger.hpp>

#include <boost/assert.hpp>

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

AutomationPerformer::Ptr
AutomationPerformer::create(io::any_io_executor executor,
                            std::shared_ptr<AutomationRegistry> registry)
{
    return Ptr(new AutomationPerformer{std::move(executor), std::move(registry)});
}

AutomationPerformer::AutomationPerformer(io::any_io_executor executor,
                                         std::shared_ptr<AutomationRegistry> registry)
    : _executor{std::move(executor)}
    , _registry{std::move(registry)}
{
    BOOST_ASSERT(_registry);
}

void
AutomationPerformer::perform(const wit::Utterances& utterances)
{
    auto filteredUtterances = utterances | std::views::filter([](const wit::Utterance& u) {
                                  return u.final && !u.intents.empty();
                              });

    for (auto&& utterance : filteredUtterances) {
        const auto intentIt = mostConfidentIntent(utterance.intents);
        BOOST_ASSERT(intentIt != std::cend(utterance.intents));
        if (_registry->has(intentIt->name)) {
            perform(intentIt->name);
        } else {
            LOGE("Unable to find automation for <{}> intent", intentIt->name);
        }
    }
}

void
AutomationPerformer::perform(const std::string& intent)
{
    Automation::Ptr automation;

    if (automation = _registry->get(intent); not automation) {
        LOGE("Unable to find automation for <{}> intent", intent);
        return;
    }

    BOOST_ASSERT(automation);
    const auto id = automation->id();
    _runningList.insert({id, automation});

    const auto alias = automation->alias();
    automation->onComplete([id, alias, weakSelf = weak_from_this()](std::error_code ec) {
        if (auto self = weakSelf.lock()) {
            self->onAutomationDone(id, alias, ec);
        }
    });

    LOGI("Execute <{} ({})> automation", alias, id);
    automation->execute(_executor);
}

void
AutomationPerformer::onAutomationDone(const std::string& id,
                                      const std::string& alias,
                                      std::error_code ec)
{
    LOGI("The <{} ({})> automation is done: result<{}>", id, alias, ec.message());

    if (const auto count = _runningList.erase(id); count > 0) {
        LOGD("Remove <{} ({})> automation from running list", id, alias);
    }
}

} // namespace jar