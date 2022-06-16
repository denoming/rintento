#include "intent/Types.hpp"

#include <fmt/format.h>

namespace jar {

Intent::Intent(std::string name, float confidence)
    : _name{std::move(name)}
    , _confidence{confidence}
{
}

const std::string&
Intent::name() const
{
    return _name;
}

double
Intent::confidence() const
{
    return _confidence;
}

bool
Intent::valid() const
{
    return !_name.empty();
}

[[nodiscard]] bool
Intent::confident() const
{
    return (_confidence > kConfidentThreshold);
}

Intent::operator bool() const
{
    return valid();
}

bool
operator==(const Intent& lhs, const Intent& rhs)
{
    if (&lhs != &rhs) {
        return (lhs.name() == rhs.name());
    }
    return true;
}

std::ostream&
operator<<(std::ostream& s, const Intent& input)
{
    static const std::string_view kFormat{"name=<{}>, confident=<{}[{.3f}]>"};
    if (input) {
        s << fmt::format(kFormat, input.name(), input.confident(), input.confidence());
    } else {
        s << "name<?>";
    }
    return s;
}

} // namespace jar