#pragma once

#include "intent/Types.hpp"

namespace jar {

class IPositioningClient {
public:
    virtual ~IPositioningClient() = default;

    [[nodiscard]] virtual GeoLocation
    location()
        = 0;
};

} // namespace jar