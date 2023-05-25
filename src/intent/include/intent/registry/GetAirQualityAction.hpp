#pragma once

#include "intent/Action.hpp"
#include "intent/WitTypes.hpp"

#include <jarvis/speaker/ISpeakerClient.hpp>
#include <jarvis/weather/Grades.hpp>
#include <jarvis/weather/IWeatherClient.hpp>

#include <memory>
#include <optional>

namespace jar {

class IPositioningClient;

class GetAirQualityAction final : public Action,
                                  public std::enable_shared_from_this<GetAirQualityAction> {
public:
    using Result = std::optional<AirQualityIndex>;

    static std::shared_ptr<GetAirQualityAction>
    create(std::string intent,
           IPositioningClient& positioningClient,
           ISpeakerClient& speakerClient,
           IWeatherClient& weatherClient,
           wit::Entities entities = {});

    [[nodiscard]] const Result&
    result() const;

    std::shared_ptr<Action>
    clone(wit::Entities entities) final;

    void
    perform() final;

private:
    GetAirQualityAction(std::string intent,
                        IPositioningClient& positioningClient,
                        ISpeakerClient& speakerClient,
                        IWeatherClient& weatherClient,
                        wit::Entities entities = {});

    void
    onAirQualityDataReady(AirQualityData data);

    void
    onAirQualityDataReady(AirQualityForecastData data);

    void
    onAirQualityDataError(std::runtime_error error);

    void
    retrieveResult(const AirQualityData& airQuality);

    void
    retrieveResult(const AirQualityForecastData& airQuality);

    void
    setResult(Result result);

    void
    announceResult();

private:
    IPositioningClient& _positioningClient;
    ISpeakerClient& _speakerClient;
    IWeatherClient& _weatherClient;
    wit::DateTimeEntity _dateTimeEntity;
    Result _result;
};

} // namespace jar