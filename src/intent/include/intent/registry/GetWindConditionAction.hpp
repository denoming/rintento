#pragma once

#include "intent/WitTypes.hpp"
#include "intent/registry/DateTimeAction.hpp"

#include <jarvis/speaker/ISpeakerClient.hpp>
#include <jarvis/weather/Grades.hpp>
#include <jarvis/weather/IWeatherClient.hpp>

#include <memory>
#include <optional>

namespace jar {

class IPositioningClient;

class GetWindConditionAction final : public DateTimeAction,
                                     public std::enable_shared_from_this<GetWindConditionAction> {
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

    void
    perform() final;

private:
    GetWindConditionAction(std::string name,
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
    retrieveResult(const CurrentWeatherData& weather);

    void
    retrieveResult(const ForecastWeatherData& weather);

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