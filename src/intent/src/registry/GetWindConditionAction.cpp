#include "intent/registry/GetWindConditionAction.hpp"

#include "intent/Formatters.hpp"
#include "intent/IPositioningClient.hpp"
#include "intent/WitHelpers.hpp"

#include <jarvis/Logger.hpp>
#include <jarvis/weather/Formatters.hpp>

#include <boost/assert.hpp>

#include <algorithm>
#include <ranges>

namespace fmt {

template<>
struct formatter<jar::GetWindConditionAction::WindValues> : public formatter<std::string_view> {
    template<typename FormatContext>
    auto
    format(const jar::GetWindConditionAction::WindValues& v, FormatContext& c) const
    {
        static constexpr const std::string_view kFormat{"min<{}>, avg<{}>, max<{}>"};
        return formatter<std::string_view>::format(fmt::format(kFormat, v.min, v.avg, v.max), c);
    }
};

} // namespace fmt

namespace jar {

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
        retrieveResult(weather);
    }
}

void
GetWindConditionAction::onWeatherDataReady(ForecastWeatherData weather)
{
    LOGD("[{}]: Getting forecast weather data was succeed", intent());

    if (cancelled()) {
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        retrieveResult(weather);
    }
}

void
GetWindConditionAction::onWeatherDataError(std::runtime_error error)
{
    LOGE("[{}]: Getting weather data has failed: error<{}>", intent(), error.what());

    setError(std::make_error_code(std::errc::result_out_of_range));
}

void
GetWindConditionAction::retrieveResult(const CurrentWeatherData& weather)
{
    try {
        const double windSpeed = weather.data.get<double>("wind.speed");
        setResult(WindValues{
            .min = WindGrade{windSpeed},
            .avg = WindGrade{windSpeed},
            .max = WindGrade{windSpeed},
        });
    } catch (const std::exception& e) {
        LOGE("[{}]: Getting wind condition has failed: {}", intent(), e.what());
        setError(std::make_error_code(std::errc::invalid_argument));
    }
}

void
GetWindConditionAction::retrieveResult(const ForecastWeatherData& weather)
{
    try {
        const wit::DateTimePredicate predicate{timestampFrom(), timestampTo()};
        int32_t count{};
        double sum{};
        double min{std::numeric_limits<double>::max()};
        double max{std::numeric_limits<double>::min()};
        std::ranges::for_each(weather.data | std::views::filter(predicate),
                              [&](const CustomData& d) {
                                  const double speed = d.get<double>("wind.speed");
                                  count++, sum += speed;
                                  min = std::min(min, speed);
                                  max = std::max(max, speed);
                              });
        setResult(WindValues{
            .min = WindGrade{min},
            .avg = WindGrade{sum / count},
            .max = WindGrade{max},
        });
    } catch (const std::exception& e) {
        LOGE("[{}]: Getting wind condition has failed: {}", intent(), e.what());
        setError(std::make_error_code(std::errc::invalid_argument));
    }
}

void
GetWindConditionAction::setResult(Result result)
{
    LOGD("[{}]: The wind values are available: {}", intent(), *result);

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
    _speakerClient.synthesizeSsml(fmt::format(fmt::runtime(kText), _result->avg), "en-US");
}

} // namespace jar
