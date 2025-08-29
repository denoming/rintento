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

#include "intent/AutomationPerformer.hpp"

#include "intent/Automation.hpp"
#include "intent/AutomationRegistry.hpp"

#include <jarvisto/core/Logger.hpp>

#include <boost/assert.hpp>

namespace jar {

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
AutomationPerformer::perform(const RecognitionResult& result)
{
    if (not result) {
        LOGD("Not understood recognition result is given");
        return;
    }

    Automation::Ptr automation;
    if (automation = _registry->get(result.intent); not automation) {
        LOGE("Unable to find automation for <{}> intent", result.intent);
        return;
    }

    BOOST_ASSERT(automation);
    const auto id = automation->id();
    _runningList.insert({id, automation});

    automation->onComplete(
        [id, alias = automation->alias(), weakSelf = weak_from_this()](std::error_code ec) {
            if (auto self = weakSelf.lock()) {
                self->onAutomationDone(id, alias, ec);
            }
        });

    LOGI("Execute <{} ({})> automation", automation->alias(), id);
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