#include "test/MockWeatherClient.hpp"

using namespace testing;

namespace jar {

MockWeatherClient::MockWeatherClient()
{
    ON_CALL(*this, getCurrentAirQuality)
        .WillByDefault(InvokeArgument<3>(std::runtime_error{"Not supported"}));

    ON_CALL(*this, getForecastAirQuality)
        .WillByDefault(InvokeArgument<3>(std::runtime_error{"Not supported"}));

    ON_CALL(*this, getCurrentUvIndex)
        .WillByDefault(InvokeArgument<3>(std::runtime_error{"Not supported"}));

    ON_CALL(*this, getForecastUvIndex)
        .WillByDefault(InvokeArgument<3>(std::runtime_error{"Not supported"}));

    ON_CALL(*this, getCurrentWeather)
        .WillByDefault(InvokeArgument<3>(std::runtime_error{"Not supported"}));

    ON_CALL(*this, getForecastWeather)
        .WillByDefault(InvokeArgument<3>(std::runtime_error{"Not supported"}));
}

} // namespace jar
