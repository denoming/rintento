#include "intent/registry/GetUvSafeTimeAction.hpp"

#include "intent/IPositioningClient.hpp"
#include "intent/WitHelpers.hpp"

#include <jarvis/Logger.hpp>

#include <boost/assert.hpp>
#include <fmt/core.h>

namespace fmt {

template<>
struct formatter<jar::GetUvSafeTimeAction::SafeExposureTime> : public formatter<std::string_view> {
    template<typename FormatContext>
    auto
    format(const jar::GetUvSafeTimeAction::SafeExposureTime& t, FormatContext& c) const
    {
        static constexpr const std::string_view kFormat{"skinType<{}>, time<{}>"};
        return formatter<std::string_view>::format(fmt::format(kFormat, t.skinType, t.time), c);
    }
};

} // namespace fmt

namespace jar {

std::shared_ptr<GetUvSafeTimeAction>
GetUvSafeTimeAction::create(std::string intent,
                            IPositioningClient& positioningClient,
                            ISpeakerClient& speakerClient,
                            IWeatherClient& weatherClient,
                            wit::Entities entities)
{
    return std::shared_ptr<GetUvSafeTimeAction>(new GetUvSafeTimeAction{
        std::move(intent), positioningClient, speakerClient, weatherClient, std::move(entities)});
}

GetUvSafeTimeAction::GetUvSafeTimeAction(std::string intent,
                                         IPositioningClient& positioningClient,
                                         ISpeakerClient& speakerClient,
                                         IWeatherClient& weatherClient,
                                         wit::Entities entities)
    : UvIndexAction{
        std::move(intent), positioningClient, speakerClient, weatherClient, std::move(entities)}
{
}

const GetUvSafeTimeAction::Result&
GetUvSafeTimeAction::GetUvSafeTimeAction::result() const
{
    return _result;
}

std::shared_ptr<Action>
GetUvSafeTimeAction::clone(wit::Entities entities)
{
    return create(intent(), positioning(), speaker(), weather(), std::move(entities));
}

void
GetUvSafeTimeAction::retrieveResult(const UvIndexData& uvIndex)
{
    try {
        if (ordinaryEntity().hasValue()) {
            if (const auto ordinary = *ordinaryEntity().value; (ordinary < 1) or (ordinary > 6)) {
                LOGE("Unexpected type of skin: {}", ordinary);
                setResult(std::nullopt);
            } else {
                LOGD("Get safe exposure time for <{}> type of skin", ordinary);
                int32_t time{};
                switch (ordinary) {
                case 1:
                    time = uvIndex.data.get<int32_t>("safe.st1");
                    break;
                case 2:
                    time = uvIndex.data.get<int32_t>("safe.st2");
                    break;
                case 3:
                    time = uvIndex.data.get<int32_t>("safe.st3");
                    break;
                case 4:
                    time = uvIndex.data.get<int32_t>("safe.st4");
                    break;
                case 5:
                    time = uvIndex.data.get<int32_t>("safe.st5");
                    break;
                case 6:
                    time = uvIndex.data.get<int32_t>("safe.st6");
                    break;
                }
                setResult(SafeExposureTime{
                    .skinType = ordinary,
                    .time = time,
                });
            };
        } else {
            LOGE("Missing type of skin");
            setResult(std::nullopt);
        }
    } catch (const std::exception& e) {
        LOGE("[{}]: Getting UV index value has failed: {}", intent(), e.what());
        setError(std::make_error_code(std::errc::invalid_argument));
    }
}

void
GetUvSafeTimeAction::retrieveResult(const UvIndexForecastData& uvIndex)
{
}

void
GetUvSafeTimeAction::setResult(Result result)
{
    LOGD("[{}]: The UV index values are available: {}", intent(), *result);

    _result = std::move(result);

    announceResult();

    finalize();
}

void
GetUvSafeTimeAction::announceResult()
{
    // clang-format off
    static constexpr const std::string_view kFormat{
        R"(The safe exposure time for <say-as interpret-as="ordinal">{}</say-as> type of skin is <break time="100ms"/> <say-as interpret-as="unit">{}</say-as> minutes)"
    };
    // clang-format on

    BOOST_ASSERT(_result);
    const std::string ssml = fmt::format(kFormat, _result->skinType, _result->time);

    speaker().synthesizeSsml(ssml, "en-US");
}

} // namespace jar
