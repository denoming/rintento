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

ForecastWeatherData
getWeatherData(bool willBeRainy,
               krn::sys_seconds timestampFrom,
               krn::sys_seconds timestampTo)
{
    static constexpr auto kStep = krn::hours{3};
    ForecastWeatherData output;
    while (timestampFrom < timestampTo) {
        const auto dt = krn::duration_cast<krn::seconds>(timestampFrom.time_since_epoch()).count();
        CustomData d;
        d.assign({
            {"dt", int64_t{dt}},
            {"duration", int32_t{10800}},
            {"id", int32_t{willBeRainy ? 501 : 800}},
        });
        output.data.push_back(std::move(d));
        timestampFrom += kStep;
    }
    return output;
}

ForecastWeatherData
getWeatherData(bool willBeRainy, krn::days modifier = {})
{
    auto p0 = krn::system_clock::now();
    auto p1 = modifier + krn::floor<krn::days>(p0);
    auto p2 = modifier + krn::ceil<krn::days>(p0);
    return getWeatherData(willBeRainy, p1, p2);
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
    EXPECT_CALL(weather, getForecastWeather).WillOnce(InvokeArgument<2>(weatherData));

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
    EXPECT_CALL(weather, getForecastWeather).WillOnce(InvokeArgument<2>(weatherData));

    const auto now = krn::system_clock::now();
    const krn::sys_seconds timestampFrom = krn::ceil<krn::hours>(now) + krn::hours{1};
    const krn::sys_seconds timestampTo = timestampFrom + krn::hours{2};

    DateTimeEntity entity;
    entity.from = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = timestampFrom,
    };
    entity.to = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = timestampTo,
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
    EXPECT_CALL(weather, getForecastWeather).WillOnce(InvokeArgument<2>(weatherData));

    const krn::sys_seconds tsFrom = krn::ceil<krn::days>(krn::system_clock::now()) + krn::hours{1};
    const krn::sys_seconds tsTo = tsFrom + krn::hours{2};

    DateTimeEntity entity;
    entity.from = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = tsFrom,
    };
    entity.to = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = tsTo,
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
    EXPECT_CALL(weather, getCurrentWeather)
        .WillOnce(InvokeArgument<3>(std::runtime_error{"Error"}));

    auto action = GetRainyStatusAction::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(Not(std::error_code{})));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    EXPECT_EQ(action->result(), std::nullopt);
}
