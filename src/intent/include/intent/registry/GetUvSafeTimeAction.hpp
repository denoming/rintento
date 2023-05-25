#pragma once

#include "intent/registry/UvIndexAction.hpp"

#include <jarvis/speaker/ISpeakerClient.hpp>
#include <jarvis/weather/IWeatherClient.hpp>

#include <memory>

namespace jar {

class GetUvSafeTimeAction final : public UvIndexAction {
public:
    struct SafeExposureTime {
        int32_t skinType{};
        int32_t time{};
    };

    using Result = std::optional<SafeExposureTime>;

    static std::shared_ptr<GetUvSafeTimeAction>
    create(std::string intent,
           IPositioningClient& positioningClient,
           ISpeakerClient& speakerClient,
           IWeatherClient& weatherClient,
           wit::Entities entities = {});

    [[nodiscard]] const Result&
    result() const;

    std::shared_ptr<Action>
    clone(wit::Entities entities) final;

private:
    GetUvSafeTimeAction(std::string intent,
                        IPositioningClient& positioningClient,
                        ISpeakerClient& speakerClient,
                        IWeatherClient& weatherClient,
                        wit::Entities entities = {});

    void
    retrieveResult(const UvIndexData& uvIndex) final;

    void
    retrieveResult(const UvIndexForecastData& uvIndex) final;

    void
    setResult(Result result);

    void
    announceResult();

private:
    Result _result;
};

} // namespace jar