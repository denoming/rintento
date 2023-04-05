#pragma once

#include "intent/Intent.hpp"
#include "jarvis/Cancellable.hpp"
#include "jarvis/speaker/ISpeakerClient.hpp"
#include "jarvis/weather/IWeatherClient.hpp"

#include <chrono>
#include <memory>

namespace jar {

class IPositioningClient;

class GetRainyStatusIntent final : public Intent,
                                   public std::enable_shared_from_this<GetRainyStatusIntent> {
public:
    enum class Tags { isRainy, notIsRainy };

    using OnReady = void(Tags tag);
    using OnError = void(std::error_code error);

    static std::shared_ptr<GetRainyStatusIntent>
    create(std::string name,
           IPositioningClient& positioningClient,
           ISpeakerClient& speakerClient,
           IWeatherClient& weatherClient,
           std::chrono::days daysModifier = {});

    std::shared_ptr<Intent>
    clone() final;

    void
    perform(OnDone callback) final;

    void
    onReady(std::move_only_function<OnReady> callback);

    void
    onError(std::move_only_function<OnError> callback);

private:
    GetRainyStatusIntent(std::string name,
                         IPositioningClient& positioningClient,
                         ISpeakerClient& speakerClient,
                         IWeatherClient& weatherClient,
                         std::chrono::days daysModifier = {});

    bool
    getRainyStatus(const ForecastWeatherData& weather);

    void
    onWeatherDataReady(ForecastWeatherData weather);

    void
    onWeatherDataError(std::error_code error);

private:
    IPositioningClient& _positioningClient;
    ISpeakerClient& _speakerClient;
    IWeatherClient& _weatherClient;
    std::chrono::days _daysModifies;
    std::move_only_function<OnReady> _onReady;
    std::move_only_function<OnError> _onError;
    OnDone _onDone;
};

} // namespace jar