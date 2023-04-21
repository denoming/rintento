#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/PositioningClient.hpp"
#include "intent/registry/GetRainyStatusIntent.hpp"
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

class GetRainyStatusIntentTest : public Test {
public:
    const std::string kIntentName{"test_rainy_status"};

public:
    NiceMock<MockPositioningClient> positioning;
    NiceMock<MockSpeakerClient> speaker;
    NiceMock<MockWeatherClient> weather;
};

TEST_F(GetRainyStatusIntentTest, CheckIfTodayIsRainy)
{
    const auto weatherData{getWeatherData(true)};

    EXPECT_CALL(speaker, synthesizeText(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getForecastWeather).WillOnce(InvokeArgument<2>(weatherData));

    auto intent = GetRainyStatusIntent::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(intent);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    auto c = intent->onDone(onDone.AsStdFunction());
    intent->perform();
    c.disconnect();

    EXPECT_EQ(intent->result().value(), GetRainyStatusIntent::Tags::isRainy);
}

TEST_F(GetRainyStatusIntentTest, CheckIfTomorrowIsRainy)
{
    const auto weatherData{getWeatherData(true, krn::days{1})};

    EXPECT_CALL(speaker, synthesizeText(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getForecastWeather).WillOnce(InvokeArgument<2>(weatherData));

    auto intent
        = GetRainyStatusIntent::create(kIntentName, positioning, speaker, weather, krn::days{1});
    ASSERT_TRUE(intent);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    auto c = intent->onDone(onDone.AsStdFunction());
    intent->perform();
    c.disconnect();

    EXPECT_EQ(intent->result().value(), GetRainyStatusIntent::Tags::isRainy);
}

TEST_F(GetRainyStatusIntentTest, CheckNotIsRainy)
{
    const auto weatherData{getWeatherData(false)};

    EXPECT_CALL(speaker, synthesizeText(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getForecastWeather).WillOnce(InvokeArgument<2>(weatherData));

    auto intent = GetRainyStatusIntent::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(intent);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    auto c = intent->onDone(onDone.AsStdFunction());
    intent->perform();
    c.disconnect();
    EXPECT_EQ(intent->result().value(), GetRainyStatusIntent::Tags::notIsRainy);
}

TEST_F(GetRainyStatusIntentTest, Error)
{
    EXPECT_CALL(weather, getForecastWeather)
        .WillOnce(InvokeArgument<3>(std::runtime_error{"Error"}));

    auto intent = GetRainyStatusIntent::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(intent);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(Not(std::error_code{})));
    auto c = intent->onDone(onDone.AsStdFunction());
    intent->perform();
    c.disconnect();

    EXPECT_THAT(intent->result().error(), Not(std::error_code{}));
}