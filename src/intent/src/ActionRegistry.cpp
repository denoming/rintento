#include "intent/ActionRegistry.hpp"

#include "intent/Action.hpp"

#include <boost/assert.hpp>

#include <stdexcept>

namespace jar {

bool
ActionRegistry::has(const std::string& intent) const
{
    return _actions.contains(intent);
}

std::shared_ptr<Action>
ActionRegistry::get(const std::string& intent, Entities entities)
{
    BOOST_ASSERT(!intent.empty());
    if (auto actionIt = _actions.find(intent); actionIt == _actions.cend()) {
        throw std::runtime_error{"No action for given intent"};
    } else {
        return std::get<1>(*actionIt)->clone(std::move(entities));
    }
}

void
ActionRegistry::add(std::shared_ptr<Action> action)
{
    BOOST_ASSERT(action);
    const auto intent{action->intent()};

    if (has(intent)) {
        throw std::runtime_error{"Action for given intent is already present"};
    } else {
        _actions.emplace(intent, std::move(action));
    }
}

void
ActionRegistry::remove(const std::string& intent)
{
    BOOST_ASSERT(!intent.empty());
    _actions.erase(intent);
}

} // namespace jar