#include "intent/registry/GetRainyStatusAction.hpp"

#include "intent/IPositioningClient.hpp"
#include "intent/WitHelpers.hpp"

#include <jarvis/Logger.hpp>

#include <boost/assert.hpp>
#include <fmt/core.h>

#include <algorithm>
#include <ranges>

namespace fmt {

template<>
struct formatter<jar::GetRainyStatusAction::RainyStatus> : public formatter<std::string_view> {
    template<typename FormatContext>
    auto
    format(const jar::GetRainyStatusAction::RainyStatus& s, FormatContext& c) const
    {
        std::string_view name{"Unknown"};
        switch (s) {
        case jar::GetRainyStatusAction::Rainy:
            name = "Rainy";
            break;
        case jar::GetRainyStatusAction::NotRainy:
            name = "NotRainy";
            break;
        }
        return fmt::formatter<std::string_view>::format(name, c);
    }
};

} // namespace fmt

namespace jar {

std::shared_ptr<GetRainyStatusAction>
GetRainyStatusAction::create(std::string intent,
                             IPositioningClient& positioningClient,
                             ISpeakerClient& speakerClient,
                             IWeatherClient& weatherClient,
                             Entities entities)
{
    return std::shared_ptr<GetRainyStatusAction>(new GetRainyStatusAction{
        std::move(intent), positioningClient, speakerClient, weatherClient, std::move(entities)});
}

GetRainyStatusAction::GetRainyStatusAction(std::string intent,
                                           IPositioningClient& positioningClient,
                                           ISpeakerClient& speakerClient,
                                           IWeatherClient& weatherClient,
                                           Entities entities)
    : WeatherAction{
        std::move(intent), positioningClient, speakerClient, weatherClient, std::move(entities)}
{
}

const GetRainyStatusAction::Result&
GetRainyStatusAction::result() const
{
    return _result;
}

std::shared_ptr<Action>
GetRainyStatusAction::clone(Entities entities)
{
    return create(intent(), positioning(), speaker(), weather(), std::move(entities));
}

void
GetRainyStatusAction::retrieveResult(const CurrentWeatherData& weather)
{
    try {
        const WeatherGrade grade{weather.data.get<int32_t>("id")};
        setResult((grade.value == WeatherGrade::Rain) ? RainyStatus::Rainy : RainyStatus::NotRainy);
    } catch (const std::exception& e) {
        LOGE("[{}]: Getting rainy status has failed: {}", intent(), e.what());
        setError(std::make_error_code(std::errc::invalid_argument));
    }
}

void
GetRainyStatusAction::retrieveResult(const ForecastWeatherData& weather)
{
    try {
        const wit::DateTimePredicate predicate{timestampFrom(), timestampTo()};
        const bool willBeRainy = std::ranges::any_of(
            weather.data | std::views::filter(predicate), [](const CustomData& d) {
                const WeatherGrade grade{d.get<int32_t>("id")};
                return (grade.value == WeatherGrade::Rain);
            });
        setResult(willBeRainy ? RainyStatus::Rainy : RainyStatus::NotRainy);
    } catch (const std::exception& e) {
        LOGE("[{}]: Getting rainy status has failed: {}", intent(), e.what());
        setError(std::make_error_code(std::errc::invalid_argument));
    }
}

void
GetRainyStatusAction::setResult(Result result)
{
    LOGD("[{}]: Rainy status is available: {}", intent(), result.value());

    _result = std::move(result);

    announceResult();

    finalize();
}

void
GetRainyStatusAction::announceResult()
{
    BOOST_ASSERT(_result);
    const std::string text{*_result ? "Today will be rainy" : "It won't rain today"};

    speaker().synthesizeText(text, "en-US");
}

} // namespace jar
