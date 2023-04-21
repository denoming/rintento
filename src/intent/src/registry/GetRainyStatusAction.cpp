#include "intent/registry/GetRainyStatusAction.hpp"

#include "intent/Formatters.hpp"
#include "intent/IPositioningClient.hpp"
#include "jarvis/Logger.hpp"
#include "jarvis/speaker/SpeakerClient.hpp"
#include "jarvis/weather/WeatherClient.hpp"

#include <boost/assert.hpp>

namespace jar {

namespace {

const int32_t kRainyStatusCode1 = 500;
const int32_t kRainyStatusCode2 = 599;

std::pair<int64_t, int64_t>
getTimeBoundaries(std::chrono::days modifier)
{
    namespace krn = std::chrono;
    krn::time_point p1 = krn::system_clock::now(); // Now
    krn::time_point p2 = krn::ceil<krn::days>(p1); // The end of current day
    if (modifier != krn::days::zero()) {
        p1 = krn::floor<krn::days>(p1) + modifier; // The start of current day + modifier
        p2 += modifier;                            // The end of current day + modifier
    }
    const auto s1 = krn::duration_cast<krn::seconds>(p1.time_since_epoch());
    const auto s2 = krn::duration_cast<krn::seconds>(p2.time_since_epoch());
    return std::make_pair(s1.count(), s2.count());
}

} // namespace

std::shared_ptr<GetRainyStatusAction>
GetRainyStatusAction::create(std::string intent,
                             IPositioningClient& locationProvider,
                             ISpeakerClient& speakerClient,
                             IWeatherClient& weatherClient,
                             std::chrono::days daysModifier)
{
    // clang-format off
    return std::shared_ptr<GetRainyStatusAction>(
        new GetRainyStatusAction{std::move(intent), locationProvider, speakerClient, weatherClient, daysModifier}
    );
    // clang-format on
}

GetRainyStatusAction::GetRainyStatusAction(std::string intent,
                                           IPositioningClient& positioningClient,
                                           ISpeakerClient& speakerClient,
                                           IWeatherClient& weatherClient,
                                           std::chrono::days daysModifier)
    : Action{std::move(intent)}
    , _positioningClient{positioningClient}
    , _speakerClient{speakerClient}
    , _weatherClient{weatherClient}
    , _daysModifies{daysModifier}
{
}

const GetRainyStatusAction::Result&
GetRainyStatusAction::result() const
{
    return _result;
}

std::shared_ptr<Action>
GetRainyStatusAction::clone()
{
    return create(intent(), _positioningClient, _speakerClient, _weatherClient, _daysModifies);
}

void
GetRainyStatusAction::perform()
{
    const auto location{_positioningClient.location()};
    LOGD("[{}]: Getting forecast weather for <{}> location", intent(), location);

    _weatherClient.getForecastWeather(
        location.lat,
        location.lon,
        [weakSelf = weak_from_this()](ForecastWeatherData weatherData) {
            if (auto self = weakSelf.lock()) {
                self->onWeatherDataReady(std::move(weatherData));
            }
        },
        [weakSelf = weak_from_this()](const std::runtime_error& error) {
            if (auto self = weakSelf.lock()) {
                self->onWeatherDataError(error);
            }
        });
}

bool
GetRainyStatusAction::getRainyStatus(const ForecastWeatherData& weather)
{
    const auto [tb1, tb2] = getTimeBoundaries(_daysModifies);

    bool isRainy{false};
    for (const auto& d : weather.data) {
        const auto dt = d.peek<int64_t>("dt");
        const auto id = d.peek<int32_t>("id");
        if (!dt or !id) {
            continue;
        }
        if (dt < tb1) {
            /* DT is earlier than [b1, b2] time boundaries */
            continue;
        }
        if (dt > tb2) {
            /* DT is later than [b1, b2] time boundaries (we assume data is sorted by dt) */
            break;
        }
        if (isRainy = (id >= kRainyStatusCode1 && id <= kRainyStatusCode2); isRainy) {
            break;
        }
    }
    return isRainy;
}

void
GetRainyStatusAction::onWeatherDataReady(ForecastWeatherData weather)
{
    LOGD("[{}]: Getting forecast weather data was succeed", intent());

    if (cancelled()) {
        setError(std::make_error_code(std::errc::operation_canceled));
        return;
    }

    const auto rainyStatus = getRainyStatus(weather);
    LOGD("[{}]: Rainy status is available: {}", intent(), rainyStatus);

    const std::string text{rainyStatus ? "Today will be rainy" : "It won't rain today"};
    _speakerClient.synthesizeText(text, "en-US");

    setResult(rainyStatus ? Tags::isRainy : Tags::notIsRainy);
}

void
GetRainyStatusAction::onWeatherDataError(std::runtime_error error)
{
    LOGE("[{}]: Getting forecast weather has failed: error<{}>", intent(), error.what());

    setError(std::make_error_code(std::errc::result_out_of_range));
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

} // namespace jar
