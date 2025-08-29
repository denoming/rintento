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

#include "intent/Automation.hpp"

#include "intent/Action.hpp"
#include "intent/LaunchStrategy.hpp"

#include <jarvisto/core/Logger.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace uuids = boost::uuids;

namespace {

std::string
generateId()
{
    static uuids::random_generator gen;
    uuids::uuid tag{gen()};
    return uuids::to_string(tag);
}

} // namespace

namespace jar {

Automation::Ptr
Automation::create(std::string alias,
                   std::string intent,
                   Action::List actions,
                   LaunchStrategy::Ptr launchStrategy)
{
    return Automation::Ptr{new Automation{generateId(),
                                          std::move(alias),
                                          std::move(intent),
                                          std::move(actions),
                                          std::move(launchStrategy)}};
}

Automation::Automation(std::string id,
                       std::string alias,
                       std::string intent,
                       Action::List actions,
                       LaunchStrategy::Ptr launchStrategy)
    : _id{std::move(id)}
    , _alias{std::move(alias)}
    , _intent{std::move(intent)}
    , _actions{std::move(actions)}
    , _launcher{std::move(launchStrategy)}
{
}

const std::string&
Automation::id() const
{
    return _id;
}

const std::string&
Automation::alias() const
{
    static const std::string kEmptyAlias{"unknown"};
    return _alias.empty() ? kEmptyAlias : _alias;
}

const std::string&
Automation::intent() const
{
    return _intent;
}

Automation::Ptr
Automation::clone()
{
    Action::List actions;
    std::for_each(_actions.cbegin(), _actions.cend(), [&](const Action::Ptr& action) {
        actions.push_back(action->clone());
    });
    BOOST_ASSERT(not actions.empty());

    LaunchStrategy::Ptr launcher = _launcher->clone();
    BOOST_ASSERT(launcher);

    const auto id = generateId();
    BOOST_ASSERT(not id.empty());
    return Automation::Ptr{new Automation{id, _alias, _intent, std::move(actions), launcher}};
}

void
Automation::execute(io::any_io_executor executor)
{
    BOOST_ASSERT(_launcher);
    _launcher->onComplete([weakSelf = weak_from_this()](std::error_code ec) {
        if (auto self = weakSelf.lock()) {
            self->onExecuteDone(ec);
        }
    });

    LOGI("Launch <{}> actions of <{} ({})> automation", _actions.size(), alias(), id());
    _launcher->launch(std::move(executor), _actions);
}

void
Automation::onExecuteDone(std::error_code ec)
{
    LOGI("Executing <{} ({})> automation is done: result<{}>", alias(), id(), ec.message());

    complete(ec);
}

} // namespace jar