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
 * The action class for getting humidity.
 *
 * Tag          Value, %
 * Comfortable  <= 55           (dry and comfortable)
 * Sticky       > 55 and < 65   (becoming "sticky" with muggy evenings)
 * Oppressive   >= 65           (lots of moisture in the air, becoming oppressive)
 */
class GetWeatherHumidityAction final
    : public DateTimeAction,
      public std::enable_shared_from_this<GetWeatherHumidityAction> {
public:
    enum class Tags { Comfortable, Sticky, Oppressive };

    struct Values {
        Tags min;
        Tags avg;
        Tags max;
    };

    using Result = std::optional<Values>;

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

    void
    perform() final;

private:
    GetWeatherHumidityAction(std::string name,
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
    retrieveHumidity(const CurrentWeatherData& weather);

    void
    retrieveHumidity(const ForecastWeatherData& weather);

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