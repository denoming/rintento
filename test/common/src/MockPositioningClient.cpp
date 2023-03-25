#include "test/MockPositioningClient.hpp"

using namespace testing;

namespace jar {

MockPositioningClient::MockPositioningClient()
{
    ON_CALL(*this, location)
        .WillByDefault(Return(GeoLocation{
            .lat = 46.4786265,
            .lon = 30.7553474,
            .alt = 0.0,
        }));
}

} // namespace jar