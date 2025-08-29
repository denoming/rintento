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

#include "intent/SequentLaunchStrategy.hpp"

#include <jarvisto/core/Logger.hpp>

#include <boost/assert.hpp>

namespace jar {

LaunchStrategy::Ptr
SequentLaunchStrategy::clone() const
{
    return LaunchStrategy::Ptr{new SequentLaunchStrategy(*this)};
}

void
SequentLaunchStrategy::launch(io::any_io_executor executor, Action::List actions)
{
    if (actions.empty()) {
        LOGW("Empty actions list");
        complete(std::make_error_code(std::errc::invalid_argument));
        return;
    }

    _executor = std::move(executor);
    _actions = std::move(actions);

    executeNextAction();
}

Action::Ptr
SequentLaunchStrategy::currentAction() const
{
    BOOST_ASSERT(not _actions.empty());
    return _actions[currentActionIndex()];
}

std::size_t
SequentLaunchStrategy::currentActionIndex() const
{
    BOOST_ASSERT(_currIndex >= 0);
    BOOST_ASSERT(_currIndex < _actions.size());
    return _currIndex;
}

bool
SequentLaunchStrategy::hasNextAction() const
{
    return (_nextIndex < _actions.size());
}

void
SequentLaunchStrategy::executeNextAction()
{
    BOOST_ASSERT(_nextIndex < _actions.size());
    auto nextAction = _actions[_nextIndex];
    _currIndex = _nextIndex++;

    BOOST_ASSERT(nextAction);
    nextAction->onComplete([weakSelf = weak_from_this()](const std::error_code ec) {
        if (auto self = weakSelf.lock()) {
            self->onActionDone(ec);
        }
    });

    LOGI("Execute the <{}> action", currentActionIndex() + 1);
    nextAction->execute(_executor);
}

void
SequentLaunchStrategy::onActionDone(std::error_code ec)
{
    LOGI("Executing the <{}> action is done: result<{}>", currentActionIndex() + 1, ec.message());

    if (ec) {
        complete(ec);
    } else {
        if (hasNextAction()) {
            executeNextAction();
        } else {
            complete();
        }
    }
}

} // namespace jar