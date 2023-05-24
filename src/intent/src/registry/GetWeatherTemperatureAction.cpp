#include "intent/registry/GetWeatherTemperatureAction.hpp"

#include "intent/IPositioningClient.hpp"
#include "intent/WitHelpers.hpp"

#include <jarvis/Logger.hpp>

#include <boost/assert.hpp>
#include <fmt/core.h>

#include <algorithm>
#include <ranges>

namespace fmt {

template<>
struct formatter<jar::GetWeatherTemperatureAction::TemperatureValues>
    : public formatter<std::string_view> {
    template<typename FormatContext>
    auto
    format(const jar::GetWeatherTemperatureAction::TemperatureValues& t, FormatContext& c) const
    {
        static constexpr const std::string_view kFormat{"min<{} [{}]>, avg<{} [{}]>, max<{} [{}]>"};
        return formatter<std::string_view>::format(
            fmt::format(
                kFormat, t.min, t.minFeelsLike, t.avg, t.avgFeelsLike, t.max, t.maxFeelsLike),
            c);
    }
};

} // namespace fmt

namespace jar {

std::shared_ptr<GetWeatherTemperatureAction>
GetWeatherTemperatureAction::create(std::string intent,
                                    IPositioningClient& positioningClient,
                                    ISpeakerClient& speakerClient,
                                    IWeatherClient& weatherClient,
                                    Entities entities)
{
    return std::shared_ptr<GetWeatherTemperatureAction>(new GetWeatherTemperatureAction{
        std::move(intent), positioningClient, speakerClient, weatherClient, std::move(entities)});
}

GetWeatherTemperatureAction::GetWeatherTemperatureAction(std::string intent,
                                                         IPositioningClient& positioningClient,
                                                         ISpeakerClient& speakerClient,
                                                         IWeatherClient& weatherClient,
                                                         Entities entities)
    : WeatherAction{
        std::move(intent), positioningClient, speakerClient, weatherClient, std::move(entities)}
{
}

const GetWeatherTemperatureAction::Result&
GetWeatherTemperatureAction::GetWeatherTemperatureAction::result() const
{
    return _result;
}

std::shared_ptr<Action>
GetWeatherTemperatureAction::clone(Entities entities)
{
    return create(intent(), positioning(), speaker(), weather(), std::move(entities));
}

void
GetWeatherTemperatureAction::retrieveResult(const WeatherData& weather)
{
    try {
        const int32_t t1 = std::round(weather.data.get<double>("main.temp"));
        const int32_t t2 = std::round(weather.data.get<double>("main.tempFeelsLike"));
        setResult(TemperatureValues{
            .min = t1,
            .minFeelsLike = t2,
            .avg = t1,
            .avgFeelsLike = t2,
            .max = t1,
            .maxFeelsLike = t2,
        });
    } catch (const std::exception& e) {
        LOGE("[{}]: Getting temperature value has failed: {}", intent(), e.what());
        setError(std::make_error_code(std::errc::invalid_argument));
    }
}

void
GetWeatherTemperatureAction::retrieveResult(const WeatherForecastData& weather)
{
    try {
        const wit::DateTimePredicate predicate{timestampFrom(), timestampTo()};
        int32_t count{};
        double sum{}, sumFeels{};
        double min{std::numeric_limits<double>::max()};
        double minFeels{min};
        double max{std::numeric_limits<double>::min()};
        double maxFeels{max};
        std::ranges::for_each(weather.data | std::views::filter(predicate),
                              [&](const CustomData& d) {
                                  const auto t1 = d.get<double>("main.temp");
                                  const auto t2 = d.get<double>("main.tempFeelsLike");
                                  count++, sum += t1, sumFeels += t2;
                                  min = std::min(min, t1);
                                  minFeels = std::min(minFeels, t2);
                                  max = std::max(max, t1);
                                  maxFeels = std::max(maxFeels, t2);
                              });
        if (count == 0) {
            setError(std::make_error_code(std::errc::no_message));
        } else {
            setResult(TemperatureValues{
                .min = static_cast<int32_t>(std::round(min)),
                .minFeelsLike = static_cast<int32_t>(std::round(minFeels)),
                .avg = static_cast<int32_t>(std::round(sum / count)),
                .avgFeelsLike = static_cast<int32_t>(std::round(sumFeels / count)),
                .max = static_cast<int32_t>(std::round(max)),
                .maxFeelsLike = static_cast<int32_t>(std::round(maxFeels)),
            });
        }
    } catch (const std::exception& e) {
        LOGE("[{}]: Getting temperature value has failed: {}", intent(), e.what());
        setError(std::make_error_code(std::errc::invalid_argument));
    }
}

void
GetWeatherTemperatureAction::setResult(Result result)
{
    LOGD("[{}]: The temperature values are available: {}", intent(), *result);

    _result = std::move(result);

    announceResult();

    finalize();
}

void
GetWeatherTemperatureAction::announceResult()
{
    // clang-format off
    static constexpr const std::string_view kFormat{
        R"(<speak>Current temperature is <say-as interpret-as="unit">{}</say-as> degrees celsius <break time="100ms"/> but feels like as <say-as interpret-as="unit">{}</say-as></speak>)"
    };
    // clang-format on

    BOOST_ASSERT(_result);
    const std::string ssml = fmt::format(kFormat, _result->avg, _result->avgFeelsLike);

    speaker().synthesizeSsml(ssml, "en-US");
}

} // namespace jar