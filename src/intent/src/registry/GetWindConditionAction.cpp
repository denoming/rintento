#include "intent/registry/GetWindConditionAction.hpp"

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
    : WeatherAction{
        std::move(intent), positioningClient, speakerClient, weatherClient, std::move(entities)}
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
    return create(intent(), positioning(), speaker(), weather(), std::move(entities));
}

void
GetWindConditionAction::retrieveResult(const WeatherData& weather)
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
GetWindConditionAction::retrieveResult(const WeatherForecastData& weather)
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

    finalize();
}

void
GetWindConditionAction::announceResult()
{
    // clang-format off
    static constexpr const std::string_view kFormat{
        R"(<speak>Wind condition is <break time="100ms"/>{}</speak>)"
    };
    // clang-format on

    BOOST_ASSERT(_result);
    const std::string ssml = fmt::format(kFormat, _result->avg);

    speaker().synthesizeSsml(ssml, "en-US");
}

} // namespace jar
