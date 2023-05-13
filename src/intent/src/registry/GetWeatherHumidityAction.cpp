#include "intent/registry/GetWeatherHumidityAction.hpp"

#include "intent/Formatters.hpp"
#include "intent/IPositioningClient.hpp"
#include "intent/WitHelpers.hpp"
#include "jarvis/Logger.hpp"

#include <boost/assert.hpp>

#include <algorithm>
#include <ranges>

namespace fmt {

template<>
struct formatter<jar::GetWeatherHumidityAction::Tags> : public formatter<std::string_view> {
    template<typename FormatContext>
    auto
    format(const jar::GetWeatherHumidityAction::Tags& tag, FormatContext& c) const
    {
        std::string_view name{"Unknown"};
        switch (tag) {
        case jar::GetWeatherHumidityAction::Tags::Comfortable:
            name = "Comfortable";
            break;
        case jar::GetWeatherHumidityAction::Tags::Sticky:
            name = "Comfortable";
            break;
        case jar::GetWeatherHumidityAction::Tags::Oppressive:
            name = "Oppressive";
            break;
        }
        return fmt::formatter<std::string_view>::format(name, c);
    }
};

} // namespace fmt

namespace jar {

namespace {

GetWeatherHumidityAction::Tags
toTag(int32_t humidity)
{
    using enum GetWeatherHumidityAction::Tags;
    auto tag{GetWeatherHumidityAction::Tags::Comfortable};
    if (humidity > 55 and humidity < 65) {
        return Sticky;
    } else if (humidity >= 65) {
        return Oppressive;
    }
    return tag;
}

} // namespace

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
        retrieveHumidity(weather);
    }
}

void
GetWeatherHumidityAction::onWeatherDataReady(ForecastWeatherData weather)
{
    LOGD("[{}]: Getting forecast weather data was succeed", intent());

    if (cancelled()) {
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        retrieveHumidity(weather);
    }
}

void
GetWeatherHumidityAction::onWeatherDataError(std::runtime_error error)
{
    LOGE("[{}]: Getting weather data has failed: error<{}>", intent(), error.what());

    setError(std::make_error_code(std::errc::result_out_of_range));
}

void
GetWeatherHumidityAction::retrieveHumidity(const CurrentWeatherData& weather)
{
    try {
        const auto humidity = toTag(weather.data.get<int32_t>("main.humidity"));
        setResult(Values{
            .min = humidity,
            .avg = humidity,
            .max = humidity,
        });
    } catch (const std::exception& e) {
        LOGE("[{}]: Getting humidity has failed: {}", intent(), e.what());
        setError(std::make_error_code(std::errc::invalid_argument));
    }
}

void
GetWeatherHumidityAction::retrieveHumidity(const ForecastWeatherData& weather)
{
    try {
        const wit::DateTimePredicate predicate{timestampFrom(), timestampTo()};
        int32_t count{};
        int32_t min{std::numeric_limits<int32_t>::max()};
        int32_t sum{};
        int32_t max{std::numeric_limits<int32_t>::min()};
        std::ranges::for_each(weather.data | std::views::filter(predicate),
                              [&](const CustomData& d) {
                                  const int32_t h = d.get<int32_t>("main.humidity");
                                  count++;
                                  if (min > h) {
                                      min = h;
                                  }
                                  sum += h;
                                  if (max < h) {
                                      max = h;
                                  }
                              });
        setResult(Values{
            .min = toTag(min),
            .avg = toTag(sum / count),
            .max = toTag(max),
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