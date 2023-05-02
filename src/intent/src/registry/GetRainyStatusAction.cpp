#include "intent/registry/GetRainyStatusAction.hpp"

#include "intent/Formatters.hpp"
#include "intent/IPositioningClient.hpp"
#include "intent/WitHelpers.hpp"
#include "jarvis/Logger.hpp"

#include <algorithm>
#include <ranges>

static const int32_t kRainyStatusCode1 = 500;
static const int32_t kRainyStatusCode2 = 599;

namespace fmt {

template<>
struct formatter<jar::GetRainyStatusAction::Tags> : public formatter<std::string_view> {
    template<typename FormatContext>
    auto
    format(const jar::GetRainyStatusAction::Tags& tag, FormatContext& c) const
    {
        std::string name{"Unknown"};
        switch (tag) {
        case jar::GetRainyStatusAction::Tags::Rainy:
            name = "Rainy";
            break;
        case jar::GetRainyStatusAction::Tags::NotRainy:
            name = "NotRainy";
            break;
        default:
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
    LOGD("[{}]: Getting forecast weather for <{}> location", intent(), loc);

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
        retrieveRainyStatus(weather);
    }
}

void
GetRainyStatusAction::onWeatherDataReady(ForecastWeatherData weather)
{
    LOGD("[{}]: Getting forecast weather data was succeed", intent());

    if (cancelled()) {
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        retrieveRainyStatus(weather);
    }
}

void
GetRainyStatusAction::onWeatherDataError(std::runtime_error error)
{
    LOGE("[{}]: Getting forecast weather has failed: error<{}>", intent(), error.what());

    setError(std::make_error_code(std::errc::result_out_of_range));
}

void
GetRainyStatusAction::retrieveRainyStatus(const CurrentWeatherData& weather)
{
    try {
        const auto id = weather.data.get<int32_t>("id");
        if (id >= kRainyStatusCode1 && id <= kRainyStatusCode2) {
            setResult(GetRainyStatusAction::Tags::Rainy);
        } else {
            setResult(GetRainyStatusAction::Tags::NotRainy);
        }
    } catch (const std::exception& e) {
        LOGE("[{}]: Getting rainy status has failed: {}", intent(), e.what());
        setError(std::make_error_code(std::errc::invalid_argument));
    }
}

void
GetRainyStatusAction::retrieveRainyStatus(const ForecastWeatherData& weather)
{
    try {
        const wit::DateTimePredicate predicate{timestampFrom(), timestampTo()};
        const bool willBeRainy = std::ranges::any_of(
            weather.data | std::views::filter(predicate), [](const CustomData& d) {
                const auto id = d.get<int32_t>("id");
                return (id >= kRainyStatusCode1 && id <= kRainyStatusCode2);
            });
        setResult(willBeRainy ? GetRainyStatusAction::Tags::Rainy
                              : GetRainyStatusAction::Tags::NotRainy);
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
    const std::string text{(_result == Tags::Rainy) ? "Today will be rainy"
                                                    : "It won't rain today"};
    _speakerClient.synthesizeText(text, "en-US");
}

} // namespace jar
