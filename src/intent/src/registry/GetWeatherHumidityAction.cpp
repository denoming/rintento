#include "intent/registry/GetWeatherHumidityAction.hpp"

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
struct formatter<jar::GetWeatherHumidityAction::HumidityValues>
    : public formatter<std::string_view> {
    template<typename FormatContext>
    auto
    format(const jar::GetWeatherHumidityAction::HumidityValues& t, FormatContext& c) const
    {
        static constexpr const std::string_view kFormat{"min<{}>, avg<{}>, max<{}>"};
        return formatter<std::string_view>::format(fmt::format(kFormat, t.min, t.avg, t.max), c);
    }
};

} // namespace fmt

namespace jar {

std::shared_ptr<GetWeatherHumidityAction>
GetWeatherHumidityAction::create(std::string intent,
                                 IPositioningClient& positioningClient,
                                 ISpeakerClient& speakerClient,
                                 IWeatherClient& weatherClient,
                                 Entities entities)
{
    return std::shared_ptr<GetWeatherHumidityAction>(new GetWeatherHumidityAction{
        std::move(intent), positioningClient, speakerClient, weatherClient, std::move(entities)});
}

GetWeatherHumidityAction::GetWeatherHumidityAction(std::string intent,
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

const GetWeatherHumidityAction::Result&
GetWeatherHumidityAction::GetWeatherHumidityAction::result() const
{
    return _result;
}

std::shared_ptr<Action>
GetWeatherHumidityAction::clone(Entities entities)
{
    return create(
        intent(), _positioningClient, _speakerClient, _weatherClient, std::move(entities));
}

void
GetWeatherHumidityAction::perform()
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
GetWeatherHumidityAction::onWeatherDataReady(CurrentWeatherData weather)
{
    LOGD("[{}]: Getting current weather data was succeed", intent());

    if (cancelled()) {
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        retrieveResult(weather);
    }
}

void
GetWeatherHumidityAction::onWeatherDataReady(ForecastWeatherData weather)
{
    LOGD("[{}]: Getting forecast weather data was succeed", intent());

    if (cancelled()) {
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        retrieveResult(weather);
    }
}

void
GetWeatherHumidityAction::onWeatherDataError(std::runtime_error error)
{
    LOGE("[{}]: Getting weather data has failed: error<{}>", intent(), error.what());

    setError(std::make_error_code(std::errc::result_out_of_range));
}

void
GetWeatherHumidityAction::retrieveResult(const CurrentWeatherData& weather)
{
    try {
        const auto grade = HumidityGrade{weather.data.get<int32_t>("main.humidity")};
        setResult(HumidityValues{
            .min = grade,
            .avg = grade,
            .max = grade,
        });
    } catch (const std::exception& e) {
        LOGE("[{}]: Getting humidity has failed: {}", intent(), e.what());
        setError(std::make_error_code(std::errc::invalid_argument));
    }
}

void
GetWeatherHumidityAction::retrieveResult(const ForecastWeatherData& weather)
{
    try {
        const wit::DateTimePredicate predicate{timestampFrom(), timestampTo()};
        int32_t count{};
        int32_t min{std::numeric_limits<int32_t>::max()};
        int32_t max{std::numeric_limits<int32_t>::min()};
        int32_t sum{};
        std::ranges::for_each(weather.data | std::views::filter(predicate),
                              [&](const CustomData& d) {
                                  const int32_t h = d.get<int32_t>("main.humidity");
                                  count++, sum += h;
                                  min = std::min(min, h);
                                  max = std::max(max, h);
                              });
        setResult(HumidityValues{
            .min = HumidityGrade{min},
            .avg = HumidityGrade{sum / count},
            .max = HumidityGrade{max},
        });
    } catch (const std::exception& e) {
        LOGE("[{}]: Getting humidity has failed: {}", intent(), e.what());
        setError(std::make_error_code(std::errc::invalid_argument));
    }
}

void
GetWeatherHumidityAction::setResult(Result result)
{
    LOGD("[{}]: The humidity values are available: min<{}>, avg<{}>, max<{}>",
         intent(),
         result->min,
         result->avg,
         result->max);

    _result = std::move(result);

    announceResult();

    complete({});
}

void
GetWeatherHumidityAction::setError(std::error_code errorCode)
{
    complete(errorCode);
}

void
GetWeatherHumidityAction::announceResult()
{
    // clang-format off
    static const std::string_view kText{
        R"(<speak>Humidity values are <break time="100ms"/>minimum value is <say-as interpret-as="unit">{}</say-as> percent <break time="100ms"/> average value is <say-as interpret-as="unit">{}</say-as> percent <break time="100ms"/> maximum value is <say-as interpret-as="unit">{}</say-as> percent</speak>)"
    };
    // clang-format on

    BOOST_ASSERT(_result);
    _speakerClient.synthesizeSsml(
        fmt::format(fmt::runtime(kText), _result->min, _result->avg, _result->max), "en-US");
}

} // namespace jar