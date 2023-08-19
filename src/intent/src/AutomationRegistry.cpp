#include "intent/AutomationRegistry.hpp"

#include "intent/Automation.hpp"

#include <jarvisto/Logger.hpp>

namespace jar {

void
AutomationRegistry::add(std::shared_ptr<Automation> automation)
{
    const std::lock_guard lock{_guard};
    auto intent = automation->intent();
    LOGD("Add new automation for <{}> intent", automation->intent());
    auto [it, ok] = _registry.insert({std::move(intent), std::move(automation)});
    BOOST_ASSERT(ok);
}

bool
AutomationRegistry::has(const std::string& intent) const
{
    const std::lock_guard lock{_guard};
    return _registry.contains(intent);
}

std::shared_ptr<Automation>
AutomationRegistry::get(const std::string& intent)
{
    const std::unique_lock lock{_guard};
    Automation::Ptr automation;
    if (auto autoIt = _registry.find(intent); autoIt != std::cend(_registry)) {
        LOGD("Clone automation for <{}> intent", intent);
        automation = std::get<1>(*autoIt)->clone();
    } else {
        LOGE("Unable to find automation for <{}> intent", intent);
    }
    return automation;
}

} // namespace jar