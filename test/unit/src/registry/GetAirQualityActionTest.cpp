#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/PositioningClient.hpp"
#include "intent/registry/GetAirQualityAction.hpp"
#include "test/MockPositioningClient.hpp"
#include "test/MockSpeakerClient.hpp"
#include "test/MockWeatherClient.hpp"

#include <chrono>

using namespace testing;
using namespace jar;

namespace krn = std::chrono;

namespace {

CustomDataSet
getAirQualityData(int32_t aqi, krn::sys_seconds tsFrom, krn::sys_seconds tsTo)
{
    static constexpr auto kStep = krn::hours{1};
    CustomDataSet output;
    while (tsFrom < tsTo) {
        const auto dt = krn::duration_cast<krn::seconds>(tsFrom.time_since_epoch()).count();
        CustomData d;
        d.assign({
            {"dt", int64_t{dt}},
            {"duration", int32_t{1 * 60 * 60}},
            {"aqi", aqi},
        });
        output.push_back(std::move(d));
        tsFrom += kStep;
    }
    return output;
}

} // namespace

class GetAirQualityActionTest : public Test {
public:
    const std::string kIntentName{"test_intent"};

public:
    NiceMock<MockPositioningClient> positioning;
    NiceMock<MockSpeakerClient> speaker;
    NiceMock<MockWeatherClient> weather;
};

TEST_F(GetAirQualityActionTest, CheckTodayAirQuality)
{
    const krn::sys_seconds t1 = krn::floor<krn::days>(krn::system_clock::now());
    const krn::sys_seconds t2 = krn::ceil<krn::days>(krn::system_clock::now());
    const auto dataSet{getAirQualityData(1, t1, t2)};

    ForecastAirQualityData airQuality = {.data = std::move(dataSet)};
    EXPECT_CALL(speaker, synthesizeText(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getForecastAirQuality).WillOnce(InvokeArgument<2>(airQuality));

    DateTimeEntity entity;
    entity.exact = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::day,
        .timestamp = Timestamp::now(),
    };

    auto action = GetAirQualityAction::create(kIntentName,
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

    EXPECT_THAT(action->result(), Optional(AirQualityIndex::Good));
}

TEST_F(GetAirQualityActionTest, CheckWorstAirQuality)
{
    const auto now = krn::system_clock::now();
    const auto t1 = krn::floor<krn::days>(now);
    const auto t2 = t1 + krn::hours{3};
    const auto dataSet1{getAirQualityData(1, t1, t2)};
    const auto t3 = t2;
    const auto t4 = t3 + krn::hours{3};
    const auto dataSet2{getAirQualityData(2, t3, t4)};
    const auto t5 = t4;
    const auto t6 = t5 + krn::hours{3};
    const auto dataSet3{getAirQualityData(4, t5, t6)};

    ForecastAirQualityData airQuality;
    airQuality.data.insert(std::end(airQuality.data), std::begin(dataSet1), std::end(dataSet1));
    airQuality.data.insert(std::end(airQuality.data), std::begin(dataSet2), std::end(dataSet2));
    airQuality.data.insert(std::end(airQuality.data), std::begin(dataSet3), std::end(dataSet3));
    EXPECT_CALL(speaker, synthesizeText(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getForecastAirQuality).WillOnce(InvokeArgument<2>(airQuality));

    DateTimeEntity entity;
    entity.from = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = Timestamp{krn::floor<krn::days>(now)},
    };
    entity.to = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = Timestamp{krn::ceil<krn::days>(now)},
    };

    auto action = GetAirQualityAction::create(kIntentName,
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

    EXPECT_THAT(action->result(), Optional(AirQualityIndex::Poor));
}

TEST_F(GetAirQualityActionTest, Error)
{
    EXPECT_CALL(weather, getCurrentAirQuality)
        .WillOnce(InvokeArgument<3>(std::runtime_error{"Error"}));

    auto action = GetAirQualityAction::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(Not(std::error_code{})));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    EXPECT_EQ(action->result(), std::nullopt);
}
