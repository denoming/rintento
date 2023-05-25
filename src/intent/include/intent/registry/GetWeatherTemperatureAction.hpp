#pragma once

#include "intent/registry/WeatherAction.hpp"

#include <jarvis/speaker/ISpeakerClient.hpp>
#include <jarvis/weather/IWeatherClient.hpp>

#include <expected>
#include <memory>

namespace jar {

class IPositioningClient;

class GetWeatherTemperatureAction final : public WeatherAction {
public:
    struct TemperatureValues {
        int32_t min{};
        int32_t minFeelsLike{};
        int32_t avg{};
        int32_t avgFeelsLike{};
        int32_t max{};
        int32_t maxFeelsLike{};
    };

    using Result = std::optional<TemperatureValues>;

    static std::shared_ptr<GetWeatherTemperatureAction>
    create(std::string intent,
           IPositioningClient& positioningClient,
           ISpeakerClient& speakerClient,
           IWeatherClient& weatherClient,
           wit::Entities entities = {});

    [[nodiscard]] const Result&
    result() const;

    std::shared_ptr<Action>
    clone(wit::Entities entities) final;

private:
    GetWeatherTemperatureAction(std::string intent,
                                IPositioningClient& positioningClient,
                                ISpeakerClient& speakerClient,
                                IWeatherClient& weatherClient,
                                wit::Entities entities = {});

    void
    retrieveResult(const WeatherData& weather);

    void
    retrieveResult(const WeatherForecastData& weather);

    void
    setResult(Result result);

    void
    announceResult();

private:
    Result _result;
};

} // namespace jar