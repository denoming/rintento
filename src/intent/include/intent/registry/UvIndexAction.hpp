#pragma once

#include "intent/Action.hpp"
#include "intent/WitTypes.hpp"

#include <jarvis/speaker/ISpeakerClient.hpp>
#include <jarvis/weather/IWeatherClient.hpp>

#include <memory>

namespace jar {

class IPositioningClient;

class UvIndexAction : public Action, public std::enable_shared_from_this<UvIndexAction> {
public:
    void
    perform() override;

protected:
    UvIndexAction(std::string intent,
                  IPositioningClient& positioningClient,
                  ISpeakerClient& speakerClient,
                  IWeatherClient& weatherClient,
                  wit::Entities entities);

    const wit::DateTimeEntity&
    dateTimeEntity() const;

    const wit::OrdinaryEntity&
    ordinaryEntity() const;

    IPositioningClient&
    positioning();

    IWeatherClient&
    weather();

    ISpeakerClient&
    speaker();

    virtual void
    retrieveResult(const UvIndexData& data)
        = 0;

    virtual void
    retrieveResult(const UvIndexForecastData& data)
        = 0;

private:
    void
    onUvIndexDataReady(UvIndexData data);

    void
    onUvIndexDataReady(UvIndexForecastData data);

    void
    onUvIndexDataError(std::runtime_error error);

private:
    IPositioningClient& _positioningClient;
    ISpeakerClient& _speakerClient;
    IWeatherClient& _weatherClient;
    wit::DateTimeEntity _dateTimeEntity;
    wit::OrdinaryEntity _ordinaryEntity;
};

} // namespace jar