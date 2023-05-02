#pragma once

#include "intent/WitTypes.hpp"
#include "intent/registry/DateTimeAction.hpp"
#include "jarvis/speaker/ISpeakerClient.hpp"
#include "jarvis/weather/IWeatherClient.hpp"

#include <optional>
#include <memory>

namespace jar {

class IPositioningClient;

class GetRainyStatusAction final : public DateTimeAction,
                                   public std::enable_shared_from_this<GetRainyStatusAction> {
public:
    enum class Tags { Rainy, NotRainy };

    using Result = std::optional<Tags>;

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
    retrieveRainyStatus(const CurrentWeatherData& weather);

    void
    retrieveRainyStatus(const ForecastWeatherData& weather);

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