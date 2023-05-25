#include "intent/registry/UvIndexAction.hpp"

#include "intent/WitHelpers.hpp"

#include "intent/IPositioningClient.hpp"

#include <jarvis/DateTime.hpp>
#include <jarvis/Formatters.hpp>
#include <jarvis/Logger.hpp>
#include <jarvis/Timestamp.hpp>

namespace jar {

UvIndexAction::UvIndexAction(std::string intent,
                             IPositioningClient& positioningClient,
                             ISpeakerClient& speakerClient,
                             IWeatherClient& weatherClient,
                             wit::Entities entities)
    : Action{std::move(intent)}
    , _positioningClient{positioningClient}
    , _speakerClient{speakerClient}
    , _weatherClient{weatherClient}
{
    wit::EntityGetter<wit::DateTimeEntity> getter1{entities};
    if (getter1.has()) {
        _dateTimeEntity = getter1.get();
    }
    wit::EntityGetter<wit::OrdinaryEntity> getter2{entities};
    if (getter2.has()) {
        _ordinaryEntity = getter2.get();
    }
}

void
UvIndexAction::perform()
{
    const auto loc{_positioningClient.location()};
    LOGD("[{}]: Getting weather data for <{}> location", intent(), loc);

    auto onReady = [weakSelf = weak_from_this()](auto weatherData) {
        if (auto self = weakSelf.lock()) {
            self->onUvIndexDataReady(std::move(weatherData));
        }
    };
    auto onError = [weakSelf = weak_from_this()](const std::runtime_error& error) {
        if (auto self = weakSelf.lock()) {
            self->onUvIndexDataError(error);
        }
    };

    if (dateTimeEntity().hasValue()) {
        if (dateTimeEntity().timestampFrom() == dateTimeEntity().timestampTo()) {
            const auto dateTime = formatUtcDateTime(dateTimeEntity().timestampFrom());
            LOGD("[{}]: Specific time is given: {}", intent(), dateTime);
            _weatherClient.getUvIndex(loc, dateTime, std::move(onReady), std::move(onError));
        } else {
            LOGD("[{}]: Time boundaries are given", intent());
            _weatherClient.getUvIndexForecast(loc, std::move(onReady), std::move(onError));
        }
    } else {
        const auto dateTime = formatUtcDateTime(Timestamp::now());
        LOGD("[{}]: No time boundaries are given", intent());
        _weatherClient.getUvIndex(loc, dateTime, std::move(onReady), std::move(onError));
    }
}

const wit::DateTimeEntity&
UvIndexAction::dateTimeEntity() const
{
    return _dateTimeEntity;
}

const wit::OrdinaryEntity&
UvIndexAction::ordinaryEntity() const
{
    return _ordinaryEntity;
}

IPositioningClient&
UvIndexAction::positioning()
{
    return _positioningClient;
}

IWeatherClient&
UvIndexAction::weather()
{
    return _weatherClient;
}

ISpeakerClient&
UvIndexAction::speaker()
{
    return _speakerClient;
}

void
UvIndexAction::onUvIndexDataReady(UvIndexData data)
{
    LOGD("[{}]: Getting UV index data was succeed", intent());

    if (cancelled()) {
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        retrieveResult(data);
    }
}

void
UvIndexAction::onUvIndexDataReady(UvIndexForecastData data)
{
    LOGD("[{}]: Getting UV index forecast data was succeed", intent());

    if (cancelled()) {
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        retrieveResult(data);
    }
}

void
UvIndexAction::onUvIndexDataError(std::runtime_error error)
{
    LOGE("[{}]: Getting UV index data has failed: error<{}>", intent(), error.what());

    setError(std::make_error_code(std::errc::result_out_of_range));
}

} // namespace jar