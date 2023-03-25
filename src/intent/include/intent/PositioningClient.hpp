#pragma once

#include "intent/IPositioningClient.hpp"

namespace jar {

class PositioningClient : public IPositioningClient {
public:
    GeoLocation
    location() final;
};

} // namespace jar