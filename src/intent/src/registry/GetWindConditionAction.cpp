#include "intent/registry/GetWindConditionAction.hpp"

#include "intent/Formatters.hpp"
#include "intent/IPositioningClient.hpp"
#include "intent/WitHelpers.hpp"
#include "jarvis/Logger.hpp"

#include <boost/assert.hpp>

#include <algorithm>
#include <ranges>

namespace fmt {

template<>
struct formatter<jar::GetWindConditionAction::Tags> : public formatter<std::string_view> {
    template<typename FormatContext>
    auto
    format(const jar::GetWindConditionAction::Tags& tag, FormatContext& c) const
    {
        std::string name{"Unknown"};
        switch (tag) {
        case jar::GetWindConditionAction::Tags::Calm:
            name = "Calm";
            break;
        case jar::GetWindConditionAction::Tags::LightAir:
            name = "LightAir";
            break;
        case jar::GetWindConditionAction::Tags::LightBreeze:
            name = "LightBreeze";
            break;
        case jar::GetWindConditionAction::Tags::GentleBreeze:
            name = "GentleBreeze";
            break;
        case jar::GetWindConditionAction::Tags::ModerateBreeze:
            name = "ModerateBreeze";
            break;
        case jar::GetWindConditionAction::Tags::FreshBreeze:
            name = "FreshBreeze";
            break;
        case jar::GetWindConditionAction::Tags::StrongBreeze:
            name = "StrongBreeze";
            break;
        case jar::GetWindConditionAction::Tags::NearGale:
            name = "NearGale";
            break;
        case jar::GetWindConditionAction::Tags::Gale:
            name = "Gale";
            break;
        case jar::GetWindConditionAction::Tags::StrongGale:
            name = "StrongGale";
            break;
        case jar::GetWindConditionAction::Tags::Storm:
            name = "Storm";
            break;
        case jar::GetWindConditionAction::Tags::ViolentStorm:
            name = "ViolentStorm";
            break;
        case jar::GetWindConditionAction::Tags::Hurricane:
            name = "Hurricane";
            break;
        }
        return fmt::formatter<std::string_view>::format(name, c);
    }
};

} // namespace fmt

namespace jar {

/**
 * Description      Wind Speed, km/h
 * Calm             <1
 * Light Air        1-5
 * Light Breeze     6-11
 * Gentle Breeze    12-19
 * Moderate Breeze  20-28
 * Fresh Breeze     29-38
 * Strong Breeze    38-49
 * Near Gale        50-61
 * Gale             62-74
 * Strong Gale      75-88
 * Storm            89-102
 * Violent Storm    103-117
 * Hurricane        118+
 */

namespace {

GetWindConditionAction::Tags
toTag(double windSpeed)
{
    using enum GetWindConditionAction::Tags;
    auto tag{GetWindConditionAction::Tags::Calm};
    if (windSpeed >= 1.0 and windSpeed <= 5.0) {
        tag = LightAir;
    } else if (windSpeed >= 6.0 and windSpeed <= 11.0) {
        tag = LightBreeze;
    } else if (windSpeed >= 12.0 and windSpeed <= 19.0) {
        tag = GentleBreeze;
    } else if (windSpeed >= 20.0 and windSpeed <= 28.0) {
        tag = ModerateBreeze;
    } else if (windSpeed >= 29.0 and windSpeed <= 38.0) {
        tag = FreshBreeze;
    } else if (windSpeed >= 38.0 and windSpeed <= 49.0) {
        tag = StrongBreeze;
    } else if (windSpeed >= 50.0 and windSpeed <= 61.0) {
        tag = NearGale;
    } else if (windSpeed >= 62.0 and windSpeed <= 74.0) {
        tag = Gale;
    } else if (windSpeed >= 75.0 and windSpeed <= 88.0) {
        tag = StrongGale;
    } else if (windSpeed >= 89.0 and windSpeed <= 102.0) {
        tag = Storm;
    } else if (windSpeed >= 103.0 and windSpeed <= 117.0) {
        tag = ViolentStorm;
    } else if (windSpeed >= 118.0) {
        tag = Hurricane;
    }
    return tag;
}

} // namespace

std::shared_ptr<GetWindConditionAction>
GetWindConditionAction::create(std::string intent,
                               IPositioningClient& positioningClient,
                               ISpeakerClient& speakerClient,
                               IWeatherClient& weatherClient,
                               Entities entities)
{
    return std::shared_ptr<GetWindConditionAction>(new GetWindConditionAction{
        std::move(intent), positioningClient, speakerClient, weatherClient, std::move(entities)});
}

GetWindConditionAction::GetWindConditionAction(std::string intent,
                                               IPositioningClient& positioningClient,
                                               ISpeakerClient& speakerClient,
                                               IWeatherClient& weatherClient,
                                               Entities entities)
    : DateTimeAction{std::move(intent), std::move(entities)}
    , _positioningClient{positioningClient}
    , _speakerClient{speakerClient}
    , _weatherClient{weatherClient}
{
}

const GetWindConditionAction::Result&
GetWindConditionAction::GetWindConditionAction::result() const
{
    return _result;
}

std::shared_ptr<Action>
GetWindConditionAction::clone(Entities entities)
{
    return create(
        intent(), _positioningClient, _speakerClient, _weatherClient, std::move(entities));
}

void
GetWindConditionAction::perform()
{
    const auto loc{_positioningClient.location()};
    LOGD("[{}]: Getting weather data for <{}> location", intent(), loc);

    auto onReady = [weakSelf = weak_from_this()](auto weatherData) {
        if (auto self = weakSelf.lock()) {
            self->onWeatherDataReady(std::move(weatherData));
        }
    };
    auto onError = [weakSelf = weak_from_this()](const std::runtime_error& error) {
        if (auto self = weakSelf.lock()) {
            self->onWeatherDataError(error);
        }
    };

    if (hasTimestamps()) {
        LOGD("[{}]: Time boundaries is available", intent());
        _weatherClient.getForecastWeather(loc.lat, loc.lon, std::move(onReady), std::move(onError));
    } else {
        LOGD("[{}]: No time boundaries is available", intent());
        _weatherClient.getCurrentWeather(loc.lat, loc.lon, std::move(onReady), std::move(onError));
    }
}

void
GetWindConditionAction::onWeatherDataReady(CurrentWeatherData weather)
{
    LOGD("[{}]: Getting current weather data was succeed", intent());

    if (cancelled()) {
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        retrieveWindCondition(weather);
    }
}

void
GetWindConditionAction::onWeatherDataReady(ForecastWeatherData weather)
{
    LOGD("[{}]: Getting forecast weather data was succeed", intent());

    if (cancelled()) {
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        retrieveWindCondition(weather);
    }
}

void
GetWindConditionAction::onWeatherDataError(std::runtime_error error)
{
    LOGE("[{}]: Getting weather data has failed: error<{}>", intent(), error.what());

    setError(std::make_error_code(std::errc::result_out_of_range));
}

void
GetWindConditionAction::retrieveWindCondition(const CurrentWeatherData& weather)
{
    try {
        const double speed = weather.data.get<double>("wind.speed");
        setResult(toTag(speed));
    } catch (const std::exception& e) {
        LOGE("[{}]: Getting wind condition has failed: {}", intent(), e.what());
        setError(std::make_error_code(std::errc::invalid_argument));
    }
}

void
GetWindConditionAction::retrieveWindCondition(const ForecastWeatherData& weather)
{
    try {
        const wit::DateTimePredicate predicate{timestampFrom(), timestampTo()};
        int32_t count{};
        double avgWindSpeed{};
        std::ranges::for_each(weather.data | std::views::filter(predicate),
                              [&](const CustomData& d) {
                                  count++;
                                  avgWindSpeed += d.get<double>("wind.speed");
                              });
        avgWindSpeed = std::round(avgWindSpeed / count);
        setResult(toTag(avgWindSpeed));
    } catch (const std::exception& e) {
        LOGE("[{}]: Getting wind condition has failed: {}", intent(), e.what());
        setError(std::make_error_code(std::errc::invalid_argument));
    }
}

void
GetWindConditionAction::setResult(Result result)
{
    LOGD("[{}]: The wind condition is available: {}", intent(), result.value());

    _result = std::move(result);

    announceResult();

    complete({});
}

void
GetWindConditionAction::setError(std::error_code errorCode)
{
    complete(errorCode);
}

void
GetWindConditionAction::announceResult()
{
    // clang-format off
    static const std::string_view kText{
        R"(<speak>Wind condition is <break time="100ms"/>{}</speak>)"
    };
    // clang-format on

    BOOST_ASSERT(_result);
    _speakerClient.synthesizeSsml(fmt::format(fmt::runtime(kText), *_result), "en-US");
}

} // namespace jar
