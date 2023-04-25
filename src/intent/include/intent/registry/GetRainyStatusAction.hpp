#pragma once

#include "intent/Action.hpp"
#include "intent/WitTypes.hpp"
#include "jarvis/speaker/ISpeakerClient.hpp"
#include "jarvis/weather/IWeatherClient.hpp"

#include <expected>
#include <memory>

namespace jar {

class IPositioningClient;

class GetRainyStatusAction final : public Action,
                                   public std::enable_shared_from_this<GetRainyStatusAction> {
public:
    enum class Tags { Unknown, Rainy, NotRainy };

    using Result = std::expected<Tags, std::error_code>;

    [[nodiscard]] static std::shared_ptr<GetRainyStatusAction>
    create(std::string intent,
           IPositioningClient& positioningClient,
           ISpeakerClient& speakerClient,
           IWeatherClient& weatherClient,
           Entities entities = {});

    [[nodiscard]] const Result&
    result() const;

    [[nodiscard]] std::shared_ptr<Action>
    clone(Entities entities) final;

    void
    perform() final;

private:
    GetRainyStatusAction(std::string name,
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
    processRainyStatus(std::optional<bool> status);

    void
    setResult(Tags tag);

    void
    setError(std::error_code errorCode);

    void
    retrieveTimeBoundaries();

    [[nodiscard]] bool
    hasTimeBoundaries() const;

private:
    IPositioningClient& _positioningClient;
    ISpeakerClient& _speakerClient;
    IWeatherClient& _weatherClient;
    Result _result;
    Timestamp _timestampFrom;
    Timestamp _timestampTo;
};

} // namespace jar