#pragma once

#include "intent/WitTypes.hpp"
#include "intent/registry/DateTimeAction.hpp"
#include "jarvis/speaker/ISpeakerClient.hpp"
#include "jarvis/weather/IWeatherClient.hpp"

#include <expected>
#include <memory>

namespace jar {

class IPositioningClient;

class GetWeatherTemperatureAction final
    : public DateTimeAction,
      public std::enable_shared_from_this<GetWeatherTemperatureAction> {
public:
    struct Temperature {
        int32_t temp{};
        int32_t tempFeelsLike{};
    };

    using Result = std::optional<Temperature>;

    static std::shared_ptr<GetWeatherTemperatureAction>
    create(std::string intent,
           IPositioningClient& positioningClient,
           ISpeakerClient& speakerClient,
           IWeatherClient& weatherClient,
           Entities entities = {});

    [[nodiscard]] const Result&
    result() const;

    std::shared_ptr<Action>
    clone(Entities entities) final;

    void
    perform() final;

private:
    GetWeatherTemperatureAction(std::string intent,
                                IPositioningClient& positioningClient,
                                ISpeakerClient& speakerClient,
                                IWeatherClient& weatherClient,
                                Entities entities = {});

    void
    onWeatherDataReady(CurrentWeatherData weather);

    void
    onWeatherDataReady(ForecastWeatherData weather);

    void
    onWeatherDataError(std::runtime_error error);

    void
    retrieveTemperature(const CurrentWeatherData& weather);

    void
    retrieveTemperature(const ForecastWeatherData& weather);

    void
    setResult(Result result);

    void
    setError(std::error_code errorCode);

    void
    announceResult();

private:
    IPositioningClient& _positioningClient;
    ISpeakerClient& _speakerClient;
    IWeatherClient& _weatherClient;
    Result _result;
};

} // namespace jar