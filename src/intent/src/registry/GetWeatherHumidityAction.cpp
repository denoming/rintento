#include "intent/registry/GetWeatherHumidityAction.hpp"

#include "intent/IPositioningClient.hpp"
#include "intent/WitHelpers.hpp"

#include <jarvis/Logger.hpp>
#include <jarvis/weather/Formatters.hpp>

#include <boost/assert.hpp>
#include <fmt/core.h>

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
    : WeatherAction{
        std::move(intent), positioningClient, speakerClient, weatherClient, std::move(entities)}
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
    return create(intent(), positioning(), speaker(), weather(), std::move(entities));
}

void
GetWeatherHumidityAction::retrieveResult(const WeatherData& weather)
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
GetWeatherHumidityAction::retrieveResult(const WeatherForecastData& weather)
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

    finalize();
}

void
GetWeatherHumidityAction::announceResult()
{
    // clang-format off
    static const constexpr std::string_view kFormat{
        R"(<speak>Humidity values are <break time="100ms"/>minimum value is <say-as interpret-as="unit">{}</say-as> percent <break time="100ms"/> average value is <say-as interpret-as="unit">{}</say-as> percent <break time="100ms"/> maximum value is <say-as interpret-as="unit">{}</say-as> percent</speak>)"
    };
    // clang-format on

    BOOST_ASSERT(_result);
    const std::string ssml = fmt::format(kFormat, _result->min, _result->avg, _result->max);

    speaker().synthesizeSsml(ssml, "en-US");
}

} // namespace jar