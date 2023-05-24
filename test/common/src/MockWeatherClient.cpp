#include "test/MockWeatherClient.hpp"

using namespace testing;

namespace jar {

MockWeatherClient::MockWeatherClient()
{
    ON_CALL(*this, getAirQuality)
        .WillByDefault(InvokeArgument<2>(std::runtime_error{"Not supported"}));

    ON_CALL(*this, getAirQualityForecast)
        .WillByDefault(InvokeArgument<2>(std::runtime_error{"Not supported"}));

    ON_CALL(*this, getUvIndex)
        .WillByDefault(InvokeArgument<3>(std::runtime_error{"Not supported"}));

    ON_CALL(*this, getUvIndexForecast)
        .WillByDefault(InvokeArgument<2>(std::runtime_error{"Not supported"}));

    ON_CALL(*this, getWeather)
        .WillByDefault(InvokeArgument<2>(std::runtime_error{"Not supported"}));

    ON_CALL(*this, getWeatherForecast)
        .WillByDefault(InvokeArgument<2>(std::runtime_error{"Not supported"}));
}

} // namespace jar
