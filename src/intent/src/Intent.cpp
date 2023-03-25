#include "intent/Intent.hpp"

namespace jar {

Intent::Intent(std::string name)
    : _name{std::move(name)}
{
}

const std::string&
Intent::name() const noexcept
{
    return _name;
}

} // namespace jar
