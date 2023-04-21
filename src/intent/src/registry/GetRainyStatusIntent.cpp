#include "intent/registry/GetRainyStatusIntent.hpp"

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

std::shared_ptr<GetRainyStatusIntent>
GetRainyStatusIntent::create(std::string name,
                             IPositioningClient& locationProvider,
                             ISpeakerClient& speakerClient,
                             IWeatherClient& weatherClient,
                             std::chrono::days daysModifier)
{
    // clang-format off
    return std::shared_ptr<GetRainyStatusIntent>(
        new GetRainyStatusIntent{std::move(name), locationProvider, speakerClient, weatherClient, daysModifier}
    );
    // clang-format on
}

GetRainyStatusIntent::GetRainyStatusIntent(std::string name,
                                           IPositioningClient& positioningClient,
                                           ISpeakerClient& speakerClient,
                                           IWeatherClient& weatherClient,
                                           std::chrono::days daysModifier)
    : Intent{std::move(name)}
    , _positioningClient{positioningClient}
    , _speakerClient{speakerClient}
    , _weatherClient{weatherClient}
    , _daysModifies{daysModifier}
{
}

const GetRainyStatusIntent::Result&
GetRainyStatusIntent::result() const
{
    return _result;
}

std::shared_ptr<Intent>
GetRainyStatusIntent::clone()
{
    return create(name(), _positioningClient, _speakerClient, _weatherClient, _daysModifies);
}

void
GetRainyStatusIntent::perform()
{
    const auto location{_positioningClient.location()};
    LOGD("[{}]: Getting forecast weather for <{}> location", name(), location);

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
GetRainyStatusIntent::getRainyStatus(const ForecastWeatherData& weather)
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
GetRainyStatusIntent::onWeatherDataReady(ForecastWeatherData weather)
{
    LOGD("[{}]: Getting forecast weather data was succeed", name());

    if (cancelled()) {
        setError(std::make_error_code(std::errc::operation_canceled));
        return;
    }

    const auto rainyStatus = getRainyStatus(weather);
    LOGD("[{}]: Rainy status is available: {}", name(), rainyStatus);

    const std::string text{rainyStatus ? "Today will be rainy" : "It won't rain today"};
    _speakerClient.synthesizeText(text, "en-US");

    setResult(rainyStatus ? Tags::isRainy : Tags::notIsRainy);
}

void
GetRainyStatusIntent::onWeatherDataError(std::runtime_error error)
{
    LOGE("[{}]: Getting forecast weather has failed: error<{}>", name(), error.what());

    setError(std::make_error_code(std::errc::result_out_of_range));
}

void
GetRainyStatusIntent::setResult(Tags tag)
{
    _result = tag;

    complete({});
}

void
GetRainyStatusIntent::setError(std::error_code errorCode)
{
    _result = std::unexpected(errorCode);

    complete(errorCode);
}

} // namespace jar
