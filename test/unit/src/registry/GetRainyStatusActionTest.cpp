#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/PositioningClient.hpp"
#include "intent/registry/GetRainyStatusAction.hpp"
#include "test/MockPositioningClient.hpp"
#include "test/MockSpeakerClient.hpp"
#include "test/MockWeatherClient.hpp"

#include <chrono>

#include "intent/WitHelpers.hpp"

using namespace testing;
using namespace jar;

namespace krn = std::chrono;

namespace {

WeatherForecastData
getWeatherData(bool willBeRainy, krn::sys_seconds tsFrom, krn::sys_seconds tsTo)
{
    static constexpr auto kStep = krn::hours{3};
    WeatherForecastData output;
    while (tsFrom < tsTo) {
        const auto dt = krn::duration_cast<krn::seconds>(tsFrom.time_since_epoch()).count();
        CustomData d;
        d.assign({
            {"dt", int64_t{dt}},
            {"duration", int32_t{10800}},
            {"id", int32_t{willBeRainy ? 501 : 800}},
        });
        output.data.push_back(std::move(d));
        tsFrom += kStep;
    }
    return output;
}

WeatherForecastData
getWeatherData(bool willBeRainy, krn::days modifier = {})
{
    auto now = krn::system_clock::now();
    auto ts1 = modifier + krn::floor<krn::days>(now);
    auto ts2 = modifier + krn::ceil<krn::days>(now);
    return getWeatherData(willBeRainy, ts1, ts2);
}

} // namespace

class GetRainyStatusActionTest : public Test {
public:
    const std::string kIntentName{"test_intent"};

public:
    NiceMock<MockPositioningClient> positioning;
    NiceMock<MockSpeakerClient> speaker;
    NiceMock<MockWeatherClient> weather;
};

TEST_F(GetRainyStatusActionTest, CheckNotRainyForToday)
{
    const auto weatherData{getWeatherData(false, krn::days{0})};
    EXPECT_CALL(speaker, synthesizeText(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getWeatherForecast).WillOnce(InvokeArgument<1>(weatherData));

    DateTimeEntity entity;
    entity.exact = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::day,
        .timestamp = Timestamp::now(),
    };

    auto action = GetRainyStatusAction::create(kIntentName,
                                               positioning,
                                               speaker,
                                               weather,
                                               {
                                                   {DateTimeEntity::key(), EntityList{entity}},
                                               });
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    EXPECT_EQ(action->result().value(), GetRainyStatusAction::RainyStatus::NotRainy);
}

TEST_F(GetRainyStatusActionTest, CheckIsRainyForInterval)
{
    const auto weatherData{getWeatherData(true, krn::days{0})};
    EXPECT_CALL(speaker, synthesizeText(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getWeatherForecast).WillOnce(InvokeArgument<1>(weatherData));

    const auto now = krn::ceil<krn::hours>(krn::system_clock::now());

    DateTimeEntity entity;
    entity.from = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = Timestamp{now + krn::hours{1}},
    };
    entity.to = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = Timestamp{now + krn::hours{3}},
    };

    auto action = GetRainyStatusAction::create(kIntentName,
                                               positioning,
                                               speaker,
                                               weather,
                                               {
                                                   {DateTimeEntity::key(), EntityList{entity}},
                                               });
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    EXPECT_THAT(action->result(), Optional(GetRainyStatusAction::RainyStatus::Rainy));
}

TEST_F(GetRainyStatusActionTest, CheckNotIsRainy)
{
    const auto weatherData{getWeatherData(true, krn::days{0})};
    EXPECT_CALL(speaker, synthesizeText(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getWeatherForecast).WillOnce(InvokeArgument<1>(weatherData));

    const auto now = krn::ceil<krn::days>(krn::system_clock::now());

    DateTimeEntity entity;
    entity.from = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = Timestamp{now + krn::hours{1}},
    };
    entity.to = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = Timestamp{now + krn::hours{3}},
    };

    auto action = GetRainyStatusAction::create(kIntentName,
                                               positioning,
                                               speaker,
                                               weather,
                                               {
                                                   {DateTimeEntity::key(), EntityList{entity}},
                                               });
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    EXPECT_THAT(action->result(), Optional(GetRainyStatusAction::RainyStatus::NotRainy));
}

TEST_F(GetRainyStatusActionTest, Error)
{
    EXPECT_CALL(weather, getWeather).WillOnce(InvokeArgument<2>(std::runtime_error{"Error"}));

    auto action = GetRainyStatusAction::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(Not(std::error_code{})));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    EXPECT_EQ(action->result(), std::nullopt);
}
