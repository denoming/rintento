#pragma once

#include "intent/registry/DateTimeAction.hpp"

#include <jarvis/speaker/ISpeakerClient.hpp>
#include <jarvis/weather/IWeatherClient.hpp>

#include <memory>

namespace jar {

class IPositioningClient;

class WeatherAction : public DateTimeAction, public std::enable_shared_from_this<WeatherAction> {
public:
    WeatherAction(std::string intent,
                  IPositioningClient& positioningClient,
                  ISpeakerClient& speakerClient,
                  IWeatherClient& weatherClient,
                  Entities entities);

    void
    perform() override;

protected:
    IPositioningClient&
    positioning();

    IWeatherClient&
    weather();

    ISpeakerClient&
    speaker();

    virtual void
    retrieveResult(const CurrentWeatherData& weather)
        = 0;

    virtual void
    retrieveResult(const ForecastWeatherData& weather)
        = 0;

private:
    void
    onWeatherDataReady(CurrentWeatherData weather);

    void
    onWeatherDataReady(ForecastWeatherData weather);

    void
    onWeatherDataError(std::runtime_error error);

private:
    IPositioningClient& _positioningClient;
    ISpeakerClient& _speakerClient;
    IWeatherClient& _weatherClient;
};

} // namespace jar