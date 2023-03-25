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

class GetTodayRainyStatusIntentTest : public Test {
public:
    const std::string kName{"get_today_rainy_status"};

    ForecastWeatherData
    getWeatherData(bool isRainy, std::chrono::days modifier = {});

public:
    NiceMock<MockPositioningClient> positioningClient;
    NiceMock<MockSpeakerClient> speakerClient;
    NiceMock<MockWeatherClient> weatherClient;
};

ForecastWeatherData
GetTodayRainyStatusIntentTest::getWeatherData(bool isRainy, std::chrono::days modifier)
{
    namespace krn = std::chrono;
    auto p0 = krn::system_clock::now();
    auto p1 = p0 + krn::seconds{1};
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

TEST_F(GetTodayRainyStatusIntentTest, CheckIsTodayRainy)
{
    const auto weatherData{getWeatherData(true)};

    EXPECT_CALL(speakerClient, synthesizeText(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weatherClient, getForecastWeather).WillOnce(InvokeArgument<2>(weatherData));

    auto intent
        = GetRainyStatusIntent::create(kName, positioningClient, speakerClient, weatherClient);
    ASSERT_TRUE(intent);

    MockFunction<void(GetRainyStatusIntent::Tags)> onReady;
    EXPECT_CALL(onReady, Call(GetRainyStatusIntent::Tags::isRainy));
    intent->onReady(onReady.AsStdFunction());

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    intent->perform(onDone.AsStdFunction());
}

TEST_F(GetTodayRainyStatusIntentTest, CheckIsTomorrowRainy)
{
    const auto weatherData{getWeatherData(true, krn::days{1})};

    EXPECT_CALL(speakerClient, synthesizeText(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weatherClient, getForecastWeather).WillOnce(InvokeArgument<2>(weatherData));

    auto intent = GetRainyStatusIntent::create(
        kName, positioningClient, speakerClient, weatherClient, krn::days{1});
    ASSERT_TRUE(intent);

    MockFunction<void(GetRainyStatusIntent::Tags)> onReady;
    EXPECT_CALL(onReady, Call(GetRainyStatusIntent::Tags::isRainy));
    intent->onReady(onReady.AsStdFunction());

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    intent->perform(onDone.AsStdFunction());
}

TEST_F(GetTodayRainyStatusIntentTest, CheckNotIsRainy)
{
    const auto weatherData{getWeatherData(false)};

    EXPECT_CALL(speakerClient, synthesizeText(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weatherClient, getForecastWeather).WillOnce(InvokeArgument<2>(weatherData));

    auto intent
        = GetRainyStatusIntent::create(kName, positioningClient, speakerClient, weatherClient);
    ASSERT_TRUE(intent);

    MockFunction<void(GetRainyStatusIntent::Tags)> onReady;
    EXPECT_CALL(onReady, Call(GetRainyStatusIntent::Tags::notIsRainy));
    intent->onReady(onReady.AsStdFunction());

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    intent->perform(onDone.AsStdFunction());
}

TEST_F(GetTodayRainyStatusIntentTest, Error)
{
    EXPECT_CALL(weatherClient, getForecastWeather)
        .WillOnce(InvokeArgument<3>(std::runtime_error{"Error"}));

    auto intent
        = GetRainyStatusIntent::create(kName, positioningClient, speakerClient, weatherClient);
    ASSERT_TRUE(intent);

    MockFunction<void(std::error_code)> onError;
    EXPECT_CALL(onError, Call(Not(std::error_code{})));
    intent->onError(onError.AsStdFunction());

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(Not(std::error_code{})));
    intent->perform(onDone.AsStdFunction());
}