#include "intent/WitTypes.hpp"

namespace krn = std::chrono;

namespace jar {

Timestamp::Timestamp(const int64_t v)
    : value{v}
{
}

Timestamp::Timestamp(krn::sys_seconds v)
    : value{v.time_since_epoch().count()}
{
}

Timestamp
Timestamp::zero()
{
    return Timestamp{};
}

Timestamp
Timestamp::now()
{
    const auto now = krn::system_clock::now();
    return krn::duration_cast<krn::seconds>(now.time_since_epoch()).count();
}

Timestamp::operator std::chrono::sys_seconds() const
{
    return krn::sys_seconds{krn::seconds{value}};
}

bool
operator==(const Intent& lhs, const Intent& rhs)
{
    if (&lhs != &rhs) {
        return (lhs.name == rhs.name);
    }
    return true;
}

} // namespace jar
