// Copyright 2025 Denys Asauliak
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "intent/AutomationRegistry.hpp"

#include "intent/Automation.hpp"

#include <jarvisto/core/Logger.hpp>

namespace jar {

void
AutomationRegistry::add(std::shared_ptr<Automation> automation)
{
    const std::lock_guard lock{_guard};
    auto intent = automation->intent();
    LOGD("Register <{}> automation for <{}> intent", automation->id(), automation->intent());
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
        automation = std::get<1>(*autoIt)->clone();
        LOGD("Provide <{}> automation for <{}> intent", automation->id(), intent);
    } else {
        LOGE("Unable to find automation prototype for <{}> intent", intent);
    }
    return automation;
}

} // namespace jar