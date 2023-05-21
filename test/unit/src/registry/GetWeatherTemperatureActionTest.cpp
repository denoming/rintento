#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/registry/GetWeatherTemperatureAction.hpp"
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
        {"main.temp", 15.1},
        {"main.tempFeelsLike", 16.1},
    });
    return CurrentWeatherData{.data = std::move(d)};
}

ForecastWeatherData
getForecastWeatherData(krn::sys_seconds timestampFrom, krn::sys_seconds timestampTo)
{
    static constexpr auto kStep = krn::hours{3};
    double temp{15.1};
    double tempFeelsLike{16.1};
    CustomDataSet dataSet;
    while (timestampFrom < timestampTo) {
        const auto dt = krn::duration_cast<krn::seconds>(timestampFrom.time_since_epoch()).count();
        CustomData d;
        d.assign({
            {"dt", int64_t{dt}},
            {"duration", int32_t{10800}},
            {"main.temp", temp},
            {"main.tempFeelsLike", tempFeelsLike},
        });
        dataSet.push_back(std::move(d));
        temp += 1.0;
        tempFeelsLike += 1.0;
        timestampFrom += kStep;
    }
    return ForecastWeatherData{.data = std::move(dataSet)};
}

} // namespace

class GetWeatherTemperatureActionTest : public Test {
public:
    const std::string kIntentName{"test_intent"};

public:
    NiceMock<MockPositioningClient> positioning;
    NiceMock<MockSpeakerClient> speaker;
    NiceMock<MockWeatherClient> weather;
};

TEST_F(GetWeatherTemperatureActionTest, GetCurrent)
{
    const auto weatherData{getCurrentWeatherData()};
    EXPECT_CALL(speaker, synthesizeSsml(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getCurrentWeather).WillOnce(InvokeArgument<2>(weatherData));

    auto action = GetWeatherTemperatureAction::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    const auto& result = action->result();
    ASSERT_TRUE(result);
    EXPECT_EQ(result->min, 15);
    EXPECT_EQ(result->minFeelsLike, 16);
    EXPECT_EQ(result->avg, 15);
    EXPECT_EQ(result->avgFeelsLike, 16);
    EXPECT_EQ(result->max, 15);
    EXPECT_EQ(result->maxFeelsLike, 16);
}

TEST_F(GetWeatherTemperatureActionTest, GetForPeriod)
{
    const krn::sys_seconds t1 = krn::floor<krn::days>(krn::system_clock::now());
    const krn::sys_seconds t2 = t1 + krn::days{1};

    const auto weatherData{getForecastWeatherData(t1, t2)};
    EXPECT_CALL(speaker, synthesizeSsml(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getForecastWeather).WillOnce(InvokeArgument<2>(weatherData));

    DateTimeEntity entity;
    entity.from = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = Timestamp{t1},
    };
    entity.to = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = Timestamp{t2},
    };

    auto action
        = GetWeatherTemperatureAction::create(kIntentName,
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
    EXPECT_THAT(result->min, Eq(15));
    EXPECT_THAT(result->minFeelsLike, Eq(16));
    EXPECT_THAT(result->avg, Gt(15));
    EXPECT_THAT(result->avgFeelsLike, Gt(16));
    EXPECT_THAT(result->max, Gt(15));
    EXPECT_THAT(result->maxFeelsLike, Gt(16));
}

TEST_F(GetWeatherTemperatureActionTest, Error)
{
    EXPECT_CALL(weather, getCurrentWeather)
        .WillOnce(InvokeArgument<3>(std::runtime_error{"Error"}));

    auto action = GetWeatherTemperatureAction::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(Not(std::error_code{})));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    EXPECT_EQ(action->result(), std::nullopt);
}