#pragma once

#include "intent/WitTypes.hpp"
#include "intent/registry/DateTimeAction.hpp"
#include "jarvis/speaker/ISpeakerClient.hpp"
#include "jarvis/weather/IWeatherClient.hpp"

#include <memory>
#include <optional>

namespace jar {

class IPositioningClient;

/**
 * The action class for getting wind condition.
 *
 *  Tag              Value, km/h
 *  Calm             <1
 *  Light Air        1-5
 *  Light Breeze     6-11
 *  Gentle Breeze    12-19
 *  Moderate Breeze  20-28
 *  Fresh Breeze     29-38
 *  Strong Breeze    38-49
 *  Near Gale        50-61
 *  Gale             62-74
 *  Strong Gale      75-88
 *  Storm            89-102
 *  Violent Storm    103-117
 *  Hurricane        118+
 *
 */
class GetWindConditionAction final : public DateTimeAction,
                                     public std::enable_shared_from_this<GetWindConditionAction> {
public:
    enum class Tags {
        Calm,
        LightAir,
        LightBreeze,
        GentleBreeze,
        ModerateBreeze,
        FreshBreeze,
        StrongBreeze,
        NearGale,
        Gale,
        StrongGale,
        Storm,
        ViolentStorm,
        Hurricane
    };

    using Result = std::optional<Tags>;

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
    retrieveWindCondition(const CurrentWeatherData& weather);

    void
    retrieveWindCondition(const ForecastWeatherData& weather);

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