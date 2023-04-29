#pragma once

#include "intent/Action.hpp"
#include "jarvis/speaker/ISpeakerClient.hpp"
#include "jarvis/weather/IWeatherClient.hpp"

#include <chrono>
#include <expected>

namespace jar {

class IPositioningClient;

class GetAirQualityAction final : public Action,
                                  public std::enable_shared_from_this<GetAirQualityAction> {
public:
    enum class Tags { Unknown, Good, Fair, Moderate, Poor, VeryPoor };

    using Result = std::expected<Tags, std::error_code>;

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
    setResult(Tags tag);

    void
    setError(std::error_code errorCode);

    void
    announceResult();

    void
    retrieveTimeBoundaries();

    [[nodiscard]] bool
    hasTimeBoundaries() const;

private:
    IPositioningClient& _positioningClient;
    ISpeakerClient& _speakerClient;
    IWeatherClient& _weatherClient;
    std::chrono::days _daysModifies;
    Result _result;
    Timestamp _tsFrom;
    Timestamp _tsTo;
};

} // namespace jar