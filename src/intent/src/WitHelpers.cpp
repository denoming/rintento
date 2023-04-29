#include "intent/WitHelpers.hpp"

namespace jar::wit {

WeatherDataPredicate::WeatherDataPredicate(Timestamp from, Timestamp to)
    : _from{from}
    , _to{to}
{
}

bool
WeatherDataPredicate::operator()(const CustomData& d) const
{
    bool output;
    int64_t b = d.get<int64_t>("dt");
    if (const auto dur = d.peek<int32_t>("duration"); dur) {
        int64_t e = b + *dur;
        output = (b >= _from and b < _to) || (e > _from and e <= _to) || (b <= _from and e >= _to);
    } else {
        output = (b >= _from and b <= _to);
    }
    return output;
}

} // namespace jar::wit