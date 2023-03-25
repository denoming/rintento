#include "intent/IntentRegistry.hpp"

#include <boost/assert.hpp>

#include <stdexcept>

namespace jar {

bool
IntentRegistry::has(const std::string& name) const
{
    return _intents.contains(name);
}

Intent::Ptr
IntentRegistry::get(const std::string& name)
{
    BOOST_ASSERT(!name.empty());
    if (auto intentIt = _intents.find(name); intentIt == _intents.cend()) {
        throw std::runtime_error{"No intent with given name"};
    } else {
        return std::get<Intent::Ptr>(*intentIt)->clone();
    }
}

void
IntentRegistry::add(Intent::Ptr intent)
{
    BOOST_ASSERT(intent);
    if (has(intent->name())) {
        throw std::runtime_error{"Intent is already present"};
    } else {
        _intents.emplace(intent->name(), std::move(intent));
    }
}

void
IntentRegistry::remove(const std::string& name)
{
    BOOST_ASSERT(!name.empty());
    _intents.erase(name);
}

} // namespace jar