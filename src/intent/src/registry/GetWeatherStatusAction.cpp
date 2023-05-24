#include "intent/registry/GetWeatherStatusAction.hpp"

#include "intent/IPositioningClient.hpp"

#include <jarvis/Logger.hpp>
#include <jarvis/weather/Formatters.hpp>

#include <boost/assert.hpp>

namespace jar {

std::shared_ptr<GetWeatherStatusAction>
GetWeatherStatusAction::create(std::string intent,
                               IPositioningClient& positioningClient,
                               ISpeakerClient& speakerClient,
                               IWeatherClient& weatherClient,
                               Entities entities)
{
    return std::shared_ptr<GetWeatherStatusAction>(new GetWeatherStatusAction{
        std::move(intent), positioningClient, speakerClient, weatherClient, std::move(entities)});
}

GetWeatherStatusAction::GetWeatherStatusAction(std::string intent,
                                               IPositioningClient& positioningClient,
                                               ISpeakerClient& speakerClient,
                                               IWeatherClient& weatherClient,
                                               Entities entities)
    : WeatherAction{
        std::move(intent), positioningClient, speakerClient, weatherClient, std::move(entities)}
{
}

const GetWeatherStatusAction::Result&
GetWeatherStatusAction::GetWeatherStatusAction::result() const
{
    return _result;
}

std::shared_ptr<Action>
GetWeatherStatusAction::clone(Entities entities)
{
    return create(intent(), positioning(), speaker(), weather(), std::move(entities));
}

void
GetWeatherStatusAction::retrieveResult(const WeatherData& weather)
{
    try {
        const int32_t id = weather.data.get<int32_t>("id");
        setResult(WeatherGrade{id});
    } catch (const std::exception& e) {
        LOGE("[{}]: Getting wind condition has failed: {}", intent(), e.what());
        setError(std::make_error_code(std::errc::invalid_argument));
    }
}

void
GetWeatherStatusAction::retrieveResult(const WeatherForecastData& weather)
{
}

void
GetWeatherStatusAction::setResult(Result result)
{
    LOGD("[{}]: The weather value is available: {}", intent(), *result);

    _result = std::move(result);

    announceResult();

    finalize();
}

void
GetWeatherStatusAction::announceResult()
{
    // clang-format off
    static constexpr const std::string_view kFormat{
        R"(<speak>Weather status is <break time="100ms"/>{}</speak>)"
    };
    // clang-format on

    BOOST_ASSERT(_result);
    const std::string ssml = fmt::format(kFormat, *_result);

    speaker().synthesizeSsml(ssml, "en-US");
}

} // namespace jar
