#include "intent/Types.hpp"

#include <fmt/format.h>

namespace jar {

bool
operator==(const Intent& lhs, const Intent& rhs)
{
    if (&lhs != &rhs) {
        return (lhs.name == rhs.name);
    }
    return true;
}

std::ostream&
operator<<(std::ostream& s, const Intent& input)
{
    constexpr std::string_view kFormat{"name=<{}>, confidence=<{:.3f}>"};
    s << fmt::format(fmt::runtime(kFormat), input.name, input.confidence);
    return s;
}

} // namespace jar