#include "intent/WitTypes.hpp"

namespace jar::wit {

bool
operator==(const Intent& lhs, const Intent& rhs)
{
    if (&lhs != &rhs) {
        return (lhs.name == rhs.name);
    }
    return true;
}

} // namespace jar
