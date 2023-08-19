#include "intent/SequentActionLaunchStrategy.hpp"

#include <jarvisto/Logger.hpp>

#include <boost/assert.hpp>

namespace jar {

ActionLaunchStrategy::Ptr
SequentActionLaunchStrategy::clone() const
{
    return ActionLaunchStrategy::Ptr{new SequentActionLaunchStrategy(*this)};
}

void
SequentActionLaunchStrategy::launch(Action::List actions)
{
    if (actions.empty()) {
        LOGW("Empty actions list");
        complete(std::make_error_code(std::errc::invalid_argument));
        return;
    }

    _actions = std::move(actions);

    executeNextAction();
}

void
SequentActionLaunchStrategy::reset()
{
    _currIndex = 0;
    _nextIndex = 0;

    _actions.clear();
    _actions.shrink_to_fit();
}

Action::Ptr
SequentActionLaunchStrategy::currentAction() const
{
    BOOST_ASSERT(not _actions.empty());
    return _actions[currentActionIndex()];
}

std::size_t
SequentActionLaunchStrategy::currentActionIndex() const
{
    BOOST_ASSERT(_currIndex >= 0);
    BOOST_ASSERT(_currIndex < _actions.size());
    return _currIndex;
}

bool
SequentActionLaunchStrategy::hasNextAction() const
{
    return (_nextIndex < _actions.size());
}

void
SequentActionLaunchStrategy::executeNextAction()
{
    BOOST_ASSERT(_nextIndex < _actions.size());
    auto& nextAction = _actions[_nextIndex];
    _currIndex = _nextIndex++;

    BOOST_ASSERT(nextAction);
    nextAction->onDone([weakSelf = weak_from_this()](const std::error_code ec) {
        if (auto self = weakSelf.lock()) {
            self->onActionDone(ec);
        }
    });

    LOGI("Execute <{}> action", currentActionIndex() + 1);
    nextAction->execute();
}

void
SequentActionLaunchStrategy::onActionDone(std::error_code ec)
{
    LOGI("Executing <{}> action is done: result<{}>", currentActionIndex() + 1, ec.message());

    if (ec) {
        reset();
        complete(ec);
    } else {
        if (hasNextAction()) {
            executeNextAction();
        } else {
            reset();
            complete();
        }
    }
}

} // namespace jar