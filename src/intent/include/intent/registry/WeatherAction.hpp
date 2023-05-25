#pragma once

#include "intent/Action.hpp"
#include "intent/WitTypes.hpp"

#include <jarvis/speaker/ISpeakerClient.hpp>
#include <jarvis/weather/IWeatherClient.hpp>

#include <memory>

namespace jar {

class IPositioningClient;

class WeatherAction : public Action, public std::enable_shared_from_this<WeatherAction> {
public:
    WeatherAction(std::string intent,
                  IPositioningClient& positioningClient,
                  ISpeakerClient& speakerClient,
                  IWeatherClient& weatherClient,
                  Entities entities);

    void
    perform() override;

protected:
    const DateTimeEntity&
    dateTimeEntity();

    IPositioningClient&
    positioning();

    IWeatherClient&
    weather();

    ISpeakerClient&
    speaker();

    virtual void
    retrieveResult(const WeatherData& weather)
        = 0;

    virtual void
    retrieveResult(const WeatherForecastData& weather)
        = 0;

private:
    void
    onWeatherDataReady(WeatherData weather);

    void
    onWeatherDataReady(WeatherForecastData weather);

    void
    onWeatherDataError(std::runtime_error error);

private:
    IPositioningClient& _positioningClient;
    ISpeakerClient& _speakerClient;
    IWeatherClient& _weatherClient;
    DateTimeEntity _dateTimeEntity;
};

} // namespace jar