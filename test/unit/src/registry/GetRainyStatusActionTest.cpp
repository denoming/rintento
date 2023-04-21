#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/PositioningClient.hpp"
#include "intent/registry/GetRainyStatusAction.hpp"
#include "test/MockPositioningClient.hpp"
#include "test/MockSpeakerClient.hpp"
#include "test/MockWeatherClient.hpp"

#include <chrono>

using namespace testing;
using namespace jar;

namespace krn = std::chrono;

static ForecastWeatherData
getWeatherData(bool isRainy, std::chrono::days modifier = {})
{
    auto p1 = krn::system_clock::now() + krn::seconds{1};
    auto p2 = p1 + krn::seconds{2};
    if (modifier != krn::days::zero()) {
        p1 += modifier;
        p2 += modifier;
    }
    const auto dt1 = krn::duration_cast<krn::seconds>(p1.time_since_epoch());
    const auto dt2 = krn::duration_cast<krn::seconds>(p2.time_since_epoch());

    CustomData d1;
    d1.assign({
        {"dt", int64_t{dt1.count()}},
        {"id", int32_t{801}},
    });
    CustomData d2;
    d2.assign({
        {"dt", int64_t{dt2.count()}},
        {"id", int32_t{isRainy ? 501 : 801}},
    });

    ForecastWeatherData output;
    output.data = {d1, d2};
    return output;
}

class GetRainyStatusActionTest : public Test {
public:
    const std::string kIntentName{"test_rainy_status"};

public:
    NiceMock<MockPositioningClient> positioning;
    NiceMock<MockSpeakerClient> speaker;
    NiceMock<MockWeatherClient> weather;
};

TEST_F(GetRainyStatusActionTest, CheckIfTodayIsRainy)
{
    const krn::days kMod{0};
    const auto weatherData{getWeatherData(true, kMod)};

    EXPECT_CALL(speaker, synthesizeText(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getForecastWeather).WillOnce(InvokeArgument<2>(weatherData));

    auto action = GetRainyStatusAction::create(kIntentName, positioning, speaker, weather, kMod);
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    EXPECT_EQ(action->result().value(), GetRainyStatusAction::Tags::isRainy);
}

TEST_F(GetRainyStatusActionTest, CheckIfTomorrowIsRainy)
{
    const krn::days kMod{1};
    const auto weatherData{getWeatherData(true, kMod)};

    EXPECT_CALL(speaker, synthesizeText(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getForecastWeather).WillOnce(InvokeArgument<2>(weatherData));

    auto action = GetRainyStatusAction::create(kIntentName, positioning, speaker, weather, kMod);
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    EXPECT_EQ(action->result().value(), GetRainyStatusAction::Tags::isRainy);
}

TEST_F(GetRainyStatusActionTest, CheckNotIsRainy)
{
    const auto weatherData{getWeatherData(false)};

    EXPECT_CALL(speaker, synthesizeText(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getForecastWeather).WillOnce(InvokeArgument<2>(weatherData));

    auto action = GetRainyStatusAction::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();
    EXPECT_EQ(action->result().value(), GetRainyStatusAction::Tags::notIsRainy);
}

TEST_F(GetRainyStatusActionTest, Error)
{
    EXPECT_CALL(weather, getForecastWeather)
        .WillOnce(InvokeArgument<3>(std::runtime_error{"Error"}));

    auto action = GetRainyStatusAction::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(Not(std::error_code{})));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    EXPECT_THAT(action->result().error(), Not(std::error_code{}));
}