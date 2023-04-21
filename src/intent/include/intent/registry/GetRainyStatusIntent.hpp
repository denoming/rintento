#pragma once

#include "intent/Intent.hpp"
#include "jarvis/speaker/ISpeakerClient.hpp"
#include "jarvis/weather/IWeatherClient.hpp"

#include <chrono>
#include <expected>
#include <memory>

namespace jar {

class IPositioningClient;

class GetRainyStatusIntent final : public Intent,
                                   public std::enable_shared_from_this<GetRainyStatusIntent> {
public:
    enum class Tags { isRainy, notIsRainy };

    using Result = std::expected<Tags, std::error_code>;

    static std::shared_ptr<GetRainyStatusIntent>
    create(std::string name,
           IPositioningClient& positioningClient,
           ISpeakerClient& speakerClient,
           IWeatherClient& weatherClient,
           std::chrono::days daysModifier = {});

    [[nodiscard]] const Result&
    result() const;

    std::shared_ptr<Intent>
    clone() final;

    void
    perform() final;

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
    onWeatherDataError(std::runtime_error error);

    void
    setResult(Tags tag);

    void
    setError(std::error_code errorCode);

private:
    IPositioningClient& _positioningClient;
    ISpeakerClient& _speakerClient;
    IWeatherClient& _weatherClient;
    std::chrono::days _daysModifies;
    Result _result;
};

} // namespace jar