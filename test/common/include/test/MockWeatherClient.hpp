#pragma once

#include <jarvis/weather/IWeatherClient.hpp>

#include <gmock/gmock.h>

namespace jar {

class MockWeatherClient : public IWeatherClient {
public:
    MockWeatherClient();

    MOCK_METHOD(void,
                getAirQuality,
                (const GeoLocation&,
                 std::function<void(AirQualityData)> onSuccess,
                 std::function<OnError> onError),
                (override));

    MOCK_METHOD(void,
                getAirQualityForecast,
                (const GeoLocation&,
                 std::function<void(AirQualityForecastData)> onSuccess,
                 std::function<OnError> onError),
                (override));

    MOCK_METHOD(void,
                getUvIndex,
                (const GeoLocation&,
                 const std::string& dateTime,
                 std::function<void(UvIndexData)> onSuccess,
                 std::function<OnError> onError),
                (override));

    MOCK_METHOD(void,
                getUvIndexForecast,
                (const GeoLocation&,
                 std::function<void(UvIndexForecastData)> onSuccess,
                 std::function<OnError> onError),
                (override));

    MOCK_METHOD(void,
                getWeather,
                (const GeoLocation&,
                 std::function<void(WeatherData)> onSuccess,
                 std::function<OnError> onError),
                (override));

    MOCK_METHOD(void,
                getWeatherForecast,
                (const GeoLocation&,
                 std::function<void(WeatherForecastData)> onSuccess,
                 std::function<OnError> onError),
                (override));
};

} // namespace jar