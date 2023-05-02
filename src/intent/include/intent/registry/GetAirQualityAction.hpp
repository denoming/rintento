#pragma once

#include "intent/WitTypes.hpp"
#include "intent/registry/DateTimeAction.hpp"
#include "jarvis/speaker/ISpeakerClient.hpp"
#include "jarvis/weather/IWeatherClient.hpp"

#include <optional>
#include <memory>

namespace jar {

class IPositioningClient;

class GetAirQualityAction final : public DateTimeAction,
                                  public std::enable_shared_from_this<GetAirQualityAction> {
public:
    enum class Tags { Unknown, Good, Fair, Moderate, Poor, VeryPoor };

    using Result = std::optional<Tags>;

    static std::shared_ptr<GetAirQualityAction>
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
    GetAirQualityAction(std::string intent,
                        IPositioningClient& positioningClient,
                        ISpeakerClient& speakerClient,
                        IWeatherClient& weatherClient,
                        Entities entities = {});

    void
    onAirQualityDataReady(CurrentAirQualityData data);

    void
    onAirQualityDataReady(ForecastAirQualityData data);

    void
    onAirQualityDataError(std::runtime_error error);

    void
    retrieveResult(const CurrentAirQualityData& airQuality);

    void
    retrieveResult(const ForecastAirQualityData& airQuality);

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