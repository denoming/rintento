#pragma once

#include "intent/IPositioningClient.hpp"

#include <gmock/gmock.h>

namespace jar {

class MockPositioningClient : public IPositioningClient {
public:
    MockPositioningClient();

    MOCK_METHOD(GeoLocation, location, (), (override));
};

} // namespace jar