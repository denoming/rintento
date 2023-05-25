#include "intent/registry/WeatherAction.hpp"

#include "intent/Formatters.hpp"
#include "intent/IPositioningClient.hpp"
#include "intent/WitHelpers.hpp"

#include <jarvis/Formatters.hpp>
#include <jarvis/Logger.hpp>

namespace jar {

WeatherAction::WeatherAction(std::string intent,
                             IPositioningClient& positioningClient,
                             ISpeakerClient& speakerClient,
                             IWeatherClient& weatherClient,
                             Entities entities)
    : Action{std::move(intent)}
    , _positioningClient{positioningClient}
    , _speakerClient{speakerClient}
    , _weatherClient{weatherClient}
{
    wit::EntityGetter<DateTimeEntity> getter{entities};
    if (getter.has()) {
        _dateTimeEntity = getter.get();
    }
}

void
WeatherAction::perform()
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

    if (dateTimeEntity().hasValue()) {
        LOGD("[{}]: Time boundaries is available", intent());
        _weatherClient.getWeatherForecast(loc, std::move(onReady), std::move(onError));
    } else {
        LOGD("[{}]: No time boundaries is available", intent());
        _weatherClient.getWeather(loc, std::move(onReady), std::move(onError));
    }
}

const DateTimeEntity&
WeatherAction::dateTimeEntity()
{
    return _dateTimeEntity;
}

IPositioningClient&
WeatherAction::positioning()
{
    return _positioningClient;
}

IWeatherClient&
WeatherAction::weather()
{
    return _weatherClient;
}

ISpeakerClient&
WeatherAction::speaker()
{
    return _speakerClient;
}

void
WeatherAction::onWeatherDataReady(WeatherData weather)
{
    LOGD("[{}]: Getting weather data was succeed", intent());

    if (cancelled()) {
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        retrieveResult(weather);
    }
}

void
WeatherAction::onWeatherDataReady(WeatherForecastData weather)
{
    LOGD("[{}]: Getting weather forecast data was succeed", intent());

    if (cancelled()) {
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        retrieveResult(weather);
    }
}

void
WeatherAction::onWeatherDataError(std::runtime_error error)
{
    LOGE("[{}]: Getting weather data has failed: error<{}>", intent(), error.what());

    setError(std::make_error_code(std::errc::result_out_of_range));
}

} // namespace jar