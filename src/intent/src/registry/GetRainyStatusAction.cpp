#include "intent/registry/GetRainyStatusAction.hpp"

#include "intent/Formatters.hpp"
#include "intent/IPositioningClient.hpp"
#include "intent/WitHelpers.hpp"

#include <jarvis/Logger.hpp>

#include <boost/assert.hpp>

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
    : DateTimeAction{std::move(intent), std::move(entities)}
    , _positioningClient{positioningClient}
    , _speakerClient{speakerClient}
    , _weatherClient{weatherClient}
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
    return create(
        intent(), _positioningClient, _speakerClient, _weatherClient, std::move(entities));
}

void
GetRainyStatusAction::perform()
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
GetRainyStatusAction::onWeatherDataReady(CurrentWeatherData weather)
{
    LOGD("[{}]: Getting current weather data was succeed", intent());

    if (cancelled()) {
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        retrieveResult(weather);
    }
}

void
GetRainyStatusAction::onWeatherDataReady(ForecastWeatherData weather)
{
    LOGD("[{}]: Getting forecast weather data was succeed", intent());

    if (cancelled()) {
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        retrieveResult(weather);
    }
}

void
GetRainyStatusAction::onWeatherDataError(std::runtime_error error)
{
    LOGE("[{}]: Getting forecast weather has failed: error<{}>", intent(), error.what());

    setError(std::make_error_code(std::errc::result_out_of_range));
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

    complete({});
}

void
GetRainyStatusAction::setError(std::error_code errorCode)
{
    complete(errorCode);
}

void
GetRainyStatusAction::announceResult()
{
    BOOST_ASSERT(_result);

    const std::string text{*_result ? "Today will be rainy" : "It won't rain today"};
    _speakerClient.synthesizeText(text, "en-US");
}

} // namespace jar
