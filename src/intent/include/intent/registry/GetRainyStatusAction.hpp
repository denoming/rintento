#pragma once

#include "intent/registry/WeatherAction.hpp"

#include <jarvis/speaker/ISpeakerClient.hpp>
#include <jarvis/weather/IWeatherClient.hpp>

#include <memory>
#include <optional>

namespace jar {

class IPositioningClient;

class GetRainyStatusAction final : public WeatherAction {
public:
    enum RainyStatus { Rainy, NotRainy };

    using Result = std::optional<RainyStatus>;

    [[nodiscard]] static std::shared_ptr<GetRainyStatusAction>
    create(std::string intent,
           IPositioningClient& positioningClient,
           ISpeakerClient& speakerClient,
           IWeatherClient& weatherClient,
           wit::Entities entities = {});

    [[nodiscard]] const Result&
    result() const;

    [[nodiscard]] std::shared_ptr<Action>
    clone(wit::Entities entities) final;

private:
    GetRainyStatusAction(std::string name,
                         IPositioningClient& positioningClient,
                         ISpeakerClient& speakerClient,
                         IWeatherClient& weatherClient,
                         wit::Entities entities = {});

    void
    retrieveResult(const WeatherData& weather) final;

    void
    retrieveResult(const WeatherForecastData& weather) final;

    void
    setResult(Result result);

    void
    announceResult();

private:
    Result _result;
};

} // namespace jar