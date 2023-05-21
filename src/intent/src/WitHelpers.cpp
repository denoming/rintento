#include "intent/WitHelpers.hpp"

namespace jar::wit {

DateTimePredicate::DateTimePredicate(Timestamp from, Timestamp to)
    : _from{from}
    , _to{to}
{
}

bool
DateTimePredicate::operator()(const CustomData& d) const
{
    bool rv;
    const int64_t d1 = d.get<int64_t>("dt");
    const Timestamp t1{d1};
    if (const auto duration = d.peek<int32_t>("duration"); duration) {
        const Timestamp t2{d1 + *duration};
        rv = (t1 >= _from && t1 < _to) || (t2 > _from && t2 <= _to) || (t1 <= _from && t2 >= _to);
    } else {
        rv = (t1 >= _from && t1 <= _to);
    }
    return rv;
}

} // namespace jar::wit