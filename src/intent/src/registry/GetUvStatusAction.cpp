#include "intent/registry/GetUvStatusAction.hpp"

#include "intent/IPositioningClient.hpp"
#include "intent/WitHelpers.hpp"

#include <jarvis/Logger.hpp>

#include <boost/assert.hpp>
#include <fmt/core.h>

#include <algorithm>
#include <ranges>

namespace fmt {

template<>
struct formatter<jar::GetUvStatusAction::UvIndexValues> : public formatter<std::string_view> {
    template<typename FormatContext>
    auto
    format(const jar::GetUvStatusAction::UvIndexValues& t, FormatContext& c) const
    {
        static constexpr const std::string_view kFormat{"min<{}>, avg<{}>, max<{}>"};
        return formatter<std::string_view>::format(fmt::format(kFormat, t.min, t.avg, t.max), c);
    }
};

} // namespace fmt

namespace jar {

std::shared_ptr<GetUvStatusAction>
GetUvStatusAction::create(std::string intent,
                          IPositioningClient& positioningClient,
                          ISpeakerClient& speakerClient,
                          IWeatherClient& weatherClient,
                          wit::Entities entities)
{
    return std::shared_ptr<GetUvStatusAction>(new GetUvStatusAction{
        std::move(intent), positioningClient, speakerClient, weatherClient, std::move(entities)});
}

GetUvStatusAction::GetUvStatusAction(std::string intent,
                                     IPositioningClient& positioningClient,
                                     ISpeakerClient& speakerClient,
                                     IWeatherClient& weatherClient,
                                     wit::Entities entities)
    : UvIndexAction{
        std::move(intent), positioningClient, speakerClient, weatherClient, std::move(entities)}
{
}

const GetUvStatusAction::Result&
GetUvStatusAction::GetUvStatusAction::result() const
{
    return _result;
}

std::shared_ptr<Action>
GetUvStatusAction::clone(wit::Entities entities)
{
    return create(intent(), positioning(), speaker(), weather(), std::move(entities));
}

void
GetUvStatusAction::retrieveResult(const UvIndexData& uvIndex)
{
    try {
        const auto uv1 = std::round(uvIndex.data.get<double>("uv"));
        const auto uv2 = std::round(uvIndex.data.get<double>("uvMax"));
        setResult(UvIndexValues{
            .min = uv1,
            .avg = uv1,
            .max = uv2,
        });
    } catch (const std::exception& e) {
        LOGE("[{}]: Getting UV index value has failed: {}", intent(), e.what());
        setError(std::make_error_code(std::errc::invalid_argument));
    }
}

void
GetUvStatusAction::retrieveResult(const UvIndexForecastData& uvIndex)
{
    try {
        const wit::DateTimePredicate predicate{dateTimeEntity().timestampFrom(),
                                               dateTimeEntity().timestampTo()};
        int32_t count{};
        double sum{};
        double min{std::numeric_limits<double>::max()};
        double max{std::numeric_limits<double>::min()};
        std::ranges::for_each(uvIndex.data | std::views::filter(predicate),
                              [&](const CustomData& d) {
                                  const auto uv = d.get<double>("uv");
                                  count++, sum += uv;
                                  min = std::min(min, uv);
                                  max = std::max(max, uv);
                              });
        if (count == 0) {
            setError(std::make_error_code(std::errc::no_message));
        } else {
            setResult(UvIndexValues{
                .min = std::round(min),
                .avg = std::round(sum / count),
                .max = std::round(max),
            });
        }
    } catch (const std::exception& e) {
        LOGE("[{}]: Getting UV index value has failed: {}", intent(), e.what());
        setError(std::make_error_code(std::errc::invalid_argument));
    }
}

void
GetUvStatusAction::setResult(Result result)
{
    LOGD("[{}]: The UV index values are available: {}", intent(), *result);

    _result = std::move(result);

    announceResult();

    finalize();
}

void
GetUvStatusAction::announceResult()
{
    // clang-format off
    static constexpr const std::string_view kFormat{
        R"(<speak>The <say-as interpret-as="characters">UV</say-as> index value is <say-as interpret-as="unit">{}</say-as> and the maximum value is <say-as interpret-as="unit">{}</say-as></speak>)"
    };
    // clang-format on

    BOOST_ASSERT(_result);
    const std::string ssml = fmt::format(kFormat, _result->avg, _result->max);

    speaker().synthesizeSsml(ssml, "en-US");
}

} // namespace jar