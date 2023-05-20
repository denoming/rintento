#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/registry/GetWeatherStatusAction.hpp"
#include "test/MockPositioningClient.hpp"
#include "test/MockSpeakerClient.hpp"
#include "test/MockWeatherClient.hpp"

#include <chrono>

using namespace testing;
using namespace jar;

namespace krn = std::chrono;

namespace {

CurrentWeatherData
getCurrentWeatherData(const int32_t weatherId)
{
    CustomData d;
    d.assign({
        {"id", weatherId},
    });
    return CurrentWeatherData{.data = std::move(d)};
}

} // namespace

class GetWeatherStatusActionTest : public Test {
public:
    const std::string kIntentName{"test_intent"};

public:
    NiceMock<MockPositioningClient> positioning;
    NiceMock<MockSpeakerClient> speaker;
    NiceMock<MockWeatherClient> weather;
};

TEST_F(GetWeatherStatusActionTest, GetCurrent)
{
    const auto weatherData{getCurrentWeatherData(801)};
    EXPECT_CALL(speaker, synthesizeSsml(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getCurrentWeather).WillOnce(InvokeArgument<2>(weatherData));

    auto action = GetWeatherStatusAction::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    EXPECT_THAT(action->result(), Optional(WeatherGrade::Clouds));
}

TEST_F(GetWeatherStatusActionTest, Error)
{
    EXPECT_CALL(weather, getCurrentWeather)
        .WillOnce(InvokeArgument<3>(std::runtime_error{"Error"}));

    auto action = GetWeatherStatusAction::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(Not(std::error_code{})));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    EXPECT_EQ(action->result(), std::nullopt);
}
