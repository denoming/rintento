#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/registry/GetWindConditionAction.hpp"
#include "test/MockPositioningClient.hpp"
#include "test/MockSpeakerClient.hpp"
#include "test/MockWeatherClient.hpp"

#include <chrono>

using namespace testing;
using namespace jar;

namespace krn = std::chrono;

namespace {

CurrentWeatherData
getCurrentWeatherData()
{
    CustomData d;
    d.assign({
        {"wind.speed", 4.1},
    });
    return CurrentWeatherData{.data = std::move(d)};
}

ForecastWeatherData
getForecastWeatherData(krn::sys_seconds timestampFrom, krn::sys_seconds timestampTo)
{
    static constexpr auto kStep = krn::hours{3};
    double windSpeed{4.1};
    CustomDataSet dataSet;
    while (timestampFrom < timestampTo) {
        const auto dt = krn::duration_cast<krn::seconds>(timestampFrom.time_since_epoch()).count();
        CustomData d;
        d.assign({
            {"dt", int64_t{dt}},
            {"duration", int32_t{10800}},
            {"wind.speed", windSpeed},
        });
        dataSet.push_back(std::move(d));
        windSpeed += 10.0;
        timestampFrom += kStep;
    }
    return ForecastWeatherData{.data = std::move(dataSet)};
}

} // namespace

class GetWindConditionActionTest : public Test {
public:
    const std::string kIntentName{"test_intent"};

public:
    NiceMock<MockPositioningClient> positioning;
    NiceMock<MockSpeakerClient> speaker;
    NiceMock<MockWeatherClient> weather;
};

TEST_F(GetWindConditionActionTest, GetCurrent)
{
    const auto weatherData{getCurrentWeatherData()};
    EXPECT_CALL(speaker, synthesizeSsml(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getCurrentWeather).WillOnce(InvokeArgument<2>(weatherData));

    auto action = GetWindConditionAction::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    const auto& result = action->result();
    ASSERT_TRUE(result);
    EXPECT_THAT(result, Optional(GetWindConditionAction::Tags::LightAir));
}

TEST_F(GetWindConditionActionTest, GetForPeriod)
{
    const krn::sys_seconds tsFrom = krn::floor<krn::days>(krn::system_clock::now());
    const krn::sys_seconds tsTo = tsFrom + krn::days{1};

    const auto weatherData{getForecastWeatherData(tsFrom, tsTo)};
    EXPECT_CALL(speaker, synthesizeSsml(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getForecastWeather).WillOnce(InvokeArgument<2>(weatherData));

    DateTimeEntity entity;
    entity.from = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = tsFrom,
    };
    entity.to = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = tsTo,
    };

    auto action = GetWindConditionAction::create(kIntentName,
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

    const auto& result = action->result();
    ASSERT_TRUE(result);
    EXPECT_THAT(result, Optional(GetWindConditionAction::Tags::StrongBreeze));
}

TEST_F(GetWindConditionActionTest, Error)
{
    EXPECT_CALL(weather, getCurrentWeather)
        .WillOnce(InvokeArgument<3>(std::runtime_error{"Error"}));

    auto action = GetWindConditionAction::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(Not(std::error_code{})));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    EXPECT_EQ(action->result(), std::nullopt);
}