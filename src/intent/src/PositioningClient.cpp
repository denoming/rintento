#include "intent/PositioningClient.hpp"

namespace jar {

GeoLocation
PositioningClient::location()
{
    return {
        .lat = 46.4786265,
        .lon = 30.7553474,
        .alt = 0.0,
    };
}

} // namespace jar
