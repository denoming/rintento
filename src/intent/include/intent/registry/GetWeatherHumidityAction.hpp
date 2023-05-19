#pragma once

#include "intent/WitTypes.hpp"
#include "intent/registry/WeatherAction.hpp"

#include <jarvis/speaker/ISpeakerClient.hpp>
#include <jarvis/weather/Grades.hpp>
#include <jarvis/weather/IWeatherClient.hpp>

#include <memory>
#include <optional>

namespace jar {

class IPositioningClient;

class GetWeatherHumidityAction final : public WeatherAction {
public:
    struct HumidityValues {
        HumidityGrade min;
        HumidityGrade avg;
        HumidityGrade max;
    };

    using Result = std::optional<HumidityValues>;

    [[nodiscard]] static std::shared_ptr<GetWeatherHumidityAction>
    create(std::string intent,
           IPositioningClient& positioningClient,
           ISpeakerClient& speakerClient,
           IWeatherClient& weatherClient,
           Entities entities = {});

    [[nodiscard]] const Result&
    result() const;

    [[nodiscard]] std::shared_ptr<Action>
    clone(Entities entities) final;

private:
    GetWeatherHumidityAction(std::string name,
                             IPositioningClient& positioningClient,
                             ISpeakerClient& speakerClient,
                             IWeatherClient& weatherClient,
                             Entities entities = {});

    void
    retrieveResult(const CurrentWeatherData& weather) final;

    void
    retrieveResult(const ForecastWeatherData& weather) final;

    void
    setResult(Result result);

    void
    announceResult();

private:
    Result _result;
};

} // namespace jar