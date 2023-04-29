#include "intent/registry/GetRainyStatusAction.hpp"

#include "intent/Formatters.hpp"
#include "intent/IPositioningClient.hpp"
#include "intent/WitHelpers.hpp"
#include "jarvis/Logger.hpp"
#include "jarvis/speaker/SpeakerClient.hpp"
#include "jarvis/weather/WeatherClient.hpp"

#include <boost/assert.hpp>

#include <algorithm>
#include <chrono>
#include <ranges>

namespace krn = std::chrono;
namespace ranges = std::ranges;

namespace jar {

namespace {

const int32_t kRainyStatusCode1 = 500;
const int32_t kRainyStatusCode2 = 599;

std::optional<bool>
getRainyStatus(const CurrentWeatherData& weather)
{
    std::optional<bool> status;
    try {
        const auto id = weather.data.get<int32_t>("id");
        status = (id >= kRainyStatusCode1 && id <= kRainyStatusCode2);
    } catch (const std::exception& e) {
        LOGE("Getting rainy status has failed: {}", e.what());
    }
    return status;
}

std::optional<bool>
getRainyStatus(const ForecastWeatherData& weather, Timestamp timestampFrom, Timestamp timestampTo)
{
    std::optional<bool> status;
    try {
        const wit::DateTimePredicate predicate{timestampFrom, timestampTo};
        return std::ranges::any_of(weather.data | std::views::filter(predicate),
                                   [](const CustomData& d) {
                                       const auto id = d.get<int32_t>("id");
                                       return (id >= kRainyStatusCode1 && id <= kRainyStatusCode2);
                                   });
    } catch (const std::exception& e) {
        LOGE("Getting rainy status has failed: {}", e.what());
    }
    return status;
}

} // namespace

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
    : Action{std::move(intent), std::move(entities)}
    , _positioningClient{positioningClient}
    , _speakerClient{speakerClient}
    , _weatherClient{weatherClient}
{
    retrieveTimeBoundaries();
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

    if (hasTimeBoundaries()) {
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
        processRainyStatus(getRainyStatus(weather));
    }
}

void
GetRainyStatusAction::onWeatherDataReady(ForecastWeatherData weather)
{
    LOGD("[{}]: Getting forecast weather data was succeed", intent());

    if (cancelled()) {
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        processRainyStatus(getRainyStatus(weather, _timestampFrom, _timestampTo));
    }
}

void
GetRainyStatusAction::onWeatherDataError(std::runtime_error error)
{
    LOGE("[{}]: Getting forecast weather has failed: error<{}>", intent(), error.what());

    setError(std::make_error_code(std::errc::result_out_of_range));
}

void
GetRainyStatusAction::processRainyStatus(std::optional<bool> status)
{
    if (status) {
        LOGD("[{}]: Rainy status is available: {}", intent(), *status);

        const std::string text{*status ? "Today will be rainy" : "It won't rain today"};
        _speakerClient.synthesizeText(text, "en-US");

        setResult(*status ? Tags::Rainy : Tags::NotRainy);
    } else {
        LOGD("[{}]: Rainy status is not available", intent());
        setResult(Tags::Unknown);
    }
}

void
GetRainyStatusAction::setResult(Tags tag)
{
    _result = tag;

    complete({});
}

void
GetRainyStatusAction::setError(std::error_code errorCode)
{
    _result = std::unexpected(errorCode);

    complete(errorCode);
}

void
GetRainyStatusAction::retrieveTimeBoundaries()
{
    wit::EntityPredicate<DateTimeEntity> predicate{entities()};
    if (!predicate.has()) {
        LOGD("No target entity is available");
        return;
    }

    const auto& entity = predicate.get();
    if (entity.from && entity.to) {
        _timestampFrom = entity.from->timestamp;
        _timestampTo = entity.to->timestamp;
    } else if (entity.exact) {
        _timestampFrom = _timestampTo = entity.exact->timestamp;
    } else {
        LOGE("Invalid entity content");
    }
}

bool
GetRainyStatusAction::hasTimeBoundaries() const
{
    return (_timestampFrom != Timestamp::zero() || _timestampTo != Timestamp::zero());
}

} // namespace jar
