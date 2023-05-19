#pragma once

#include "intent/WitTypes.hpp"
#include "intent/registry/WeatherAction.hpp"

#include <jarvis/speaker/ISpeakerClient.hpp>
#include <jarvis/weather/Grades.hpp>
#include <jarvis/weather/IWeatherClient.hpp>

#include <optional>

namespace jar {

class IPositioningClient;

class GetWindConditionAction final : public WeatherAction {
public:
    struct WindValues {
        WindGrade min;
        WindGrade avg;
        WindGrade max;
    };

    using Result = std::optional<WindValues>;

    [[nodiscard]] static std::shared_ptr<GetWindConditionAction>
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
    GetWindConditionAction(std::string name,
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