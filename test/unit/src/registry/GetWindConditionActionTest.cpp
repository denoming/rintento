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

WeatherData
getWeatherData()
{
    CustomData d;
    d.assign({
        {"wind.speed", 4.1},
    });
    return WeatherData{.data = std::move(d)};
}

WeatherForecastData
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
    return WeatherForecastData{.data = std::move(dataSet)};
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
    const auto weatherData{getWeatherData()};
    EXPECT_CALL(speaker, synthesizeSsml(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getWeather).WillOnce(InvokeArgument<1>(weatherData));

    auto action = GetWindConditionAction::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    const auto& result = action->result();
    ASSERT_TRUE(result);
    EXPECT_EQ(result->min, WindGrade::LightAir);
    EXPECT_EQ(result->avg, WindGrade::LightAir);
    EXPECT_EQ(result->max, WindGrade::LightAir);
}

TEST_F(GetWindConditionActionTest, GetForPeriod)
{
    const krn::sys_seconds t1 = krn::floor<krn::days>(krn::system_clock::now());
    const krn::sys_seconds t2 = t1 + krn::days{1};

    const auto weatherData{getForecastWeatherData(t1, t2)};
    EXPECT_CALL(speaker, synthesizeSsml(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getWeatherForecast).WillOnce(InvokeArgument<1>(weatherData));

    DateTimeEntity entity;
    entity.valueFrom = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = Timestamp{t1},
    };
    entity.valueTo = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = Timestamp{t2},
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
    EXPECT_THAT(result->min, Eq(WindGrade::LightAir));
    EXPECT_THAT(result->avg, Not(WindGrade::LightAir));
    EXPECT_THAT(result->max, Not(WindGrade::LightAir));
}

TEST_F(GetWindConditionActionTest, Error)
{
    EXPECT_CALL(weather, getWeather).WillOnce(InvokeArgument<2>(std::runtime_error{"Error"}));

    auto action = GetWindConditionAction::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(Not(std::error_code{})));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    EXPECT_EQ(action->result(), std::nullopt);
}