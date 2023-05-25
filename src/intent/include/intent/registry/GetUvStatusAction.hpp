#pragma once

#include "intent/registry/UvIndexAction.hpp"

#include <jarvis/speaker/ISpeakerClient.hpp>
#include <jarvis/weather/IWeatherClient.hpp>

#include <memory>

namespace jar {

class GetUvStatusAction final : public UvIndexAction {
public:
    struct UvIndexValues {
        double min{};
        double avg{};
        double max{};
    };

    using Result = std::optional<UvIndexValues>;

    static std::shared_ptr<GetUvStatusAction>
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
    GetUvStatusAction(std::string intent,
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