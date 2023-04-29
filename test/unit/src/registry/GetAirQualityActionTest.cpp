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

int32_t
fromTag(GetAirQualityAction::Tags tag)
{
    int32_t output;
    switch (tag) {
    case GetAirQualityAction::Tags::Good:
        output = 1;
        break;
    case GetAirQualityAction::Tags::Fair:
        output = 2;
        break;
    case GetAirQualityAction::Tags::Moderate:
        output = 3;
        break;
    case GetAirQualityAction::Tags::Poor:
        output = 4;
        break;
    case GetAirQualityAction::Tags::VeryPoor:
        output = 5;
        break;
    default:
        output = -1;
    }
    return output;
}

CustomDataSet
getAirQualityData(GetAirQualityAction::Tags tag, krn::sys_seconds tsFrom, krn::sys_seconds tsTo)
{
    static constexpr auto kStep = krn::hours{1};
    CustomDataSet output;
    while (tsFrom < tsTo) {
        const auto dt = krn::duration_cast<krn::seconds>(tsFrom.time_since_epoch()).count();
        CustomData d;
        d.assign({
            {"dt", int64_t{dt}},
            {"duration", int32_t{1 * 60 * 60}},
            {"aqi", int32_t{fromTag(tag)}},
        });
        output.push_back(std::move(d));
        tsFrom += kStep;
    }
    return output;
}

} // namespace

class GetAirQualityActionTest : public Test {
public:
    const std::string kIntentName{"test_rainy_status"};

public:
    NiceMock<MockPositioningClient> positioning;
    NiceMock<MockSpeakerClient> speaker;
    NiceMock<MockWeatherClient> weather;
};

TEST_F(GetAirQualityActionTest, CheckTodayAirQuality)
{
    const krn::sys_seconds tsFrom = krn::floor<krn::days>(krn::system_clock::now());
    const krn::sys_seconds tsTo = krn::ceil<krn::days>(krn::system_clock::now());
    const auto dataSet{getAirQualityData(GetAirQualityAction::Tags::Good, tsFrom, tsTo)};

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

    EXPECT_EQ(action->result().value(), GetAirQualityAction::Tags::Good);
}

TEST_F(GetAirQualityActionTest, CheckWorstAirQuality)
{
    const auto tsForm1 = krn::floor<krn::days>(krn::system_clock::now());
    const auto tsTo1 = tsForm1 + krn::hours{3};
    const auto dataSet1{getAirQualityData(GetAirQualityAction::Tags::Good, tsForm1, tsTo1)};
    const auto tsForm2 = tsTo1;
    const auto tsTo2 = tsForm2 + krn::hours{3};
    const auto dataSet2{getAirQualityData(GetAirQualityAction::Tags::Fair, tsForm2, tsTo2)};
    const auto tsForm3 = tsTo2;
    const auto tsTo3 = tsForm3 + krn::hours{3};
    const auto dataSet3{getAirQualityData(GetAirQualityAction::Tags::Poor, tsForm3, tsTo3)};

    ForecastAirQualityData airQuality;
    airQuality.data.insert(std::end(airQuality.data), std::begin(dataSet1), std::end(dataSet1));
    airQuality.data.insert(std::end(airQuality.data), std::begin(dataSet2), std::end(dataSet2));
    airQuality.data.insert(std::end(airQuality.data), std::begin(dataSet3), std::end(dataSet3));
    EXPECT_CALL(speaker, synthesizeText(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getForecastAirQuality).WillOnce(InvokeArgument<2>(airQuality));

    const krn::sys_seconds dtFrom = krn::floor<krn::days>(krn::system_clock::now());
    const krn::sys_seconds dtTo = krn::ceil<krn::days>(krn::system_clock::now());

    DateTimeEntity entity;
    entity.from = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = dtFrom,
    };
    entity.to = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = dtTo,
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

    EXPECT_EQ(action->result().value(), GetAirQualityAction::Tags::Poor);
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

    EXPECT_THAT(action->result().error(), Not(std::error_code{}));
}
