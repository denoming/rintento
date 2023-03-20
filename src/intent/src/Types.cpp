#include "intent/Types.hpp"

namespace jar {

bool
operator==(const IntentSpec& lhs, const IntentSpec& rhs)
{
    if (&lhs != &rhs) {
        return (lhs.name == rhs.name);
    }
    return true;
}

} // namespace jar