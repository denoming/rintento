#pragma once

#include <jarvis/weather/IWeatherClient.hpp>

#include <gmock/gmock.h>

namespace jar {

class MockWeatherClient : public IWeatherClient {
public:
    MockWeatherClient();

    MOCK_METHOD(void,
                getCurrentAirQuality,
                (double lat,
                 double lon,
                 std::function<void(CurrentAirQualityData)> onSuccess,
                 std::function<OnError> onError),
                (override));

    MOCK_METHOD(void,
                getForecastAirQuality,
                (double lat,
                 double lon,
                 std::function<void(ForecastAirQualityData)> onSuccess,
                 std::function<OnError> onError),
                (override));

    MOCK_METHOD(void,
                getCurrentUvIndex,
                (double lat,
                 double lon,
                 std::function<void(CurrentUvIndexData)> onSuccess,
                 std::function<OnError> onError),
                (override));

    MOCK_METHOD(void,
                getForecastUvIndex,
                (double lat,
                 double lon,
                 std::function<void(ForecastUvIndexData)> onSuccess,
                 std::function<OnError> onError),
                (override));

    MOCK_METHOD(void,
                getCurrentWeather,
                (double lat,
                 double lon,
                 std::function<void(CurrentWeatherData)> onSuccess,
                 std::function<OnError> onError),
                (override));

    MOCK_METHOD(void,
                getForecastWeather,
                (double lat,
                 double lon,
                 std::function<void(ForecastWeatherData)> onSuccess,
                 std::function<OnError> onError),
                (override));
};

} // namespace jar