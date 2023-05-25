#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/registry/GetUvStatusAction.hpp"
#include "test/MockPositioningClient.hpp"
#include "test/MockSpeakerClient.hpp"
#include "test/MockWeatherClient.hpp"

#include <chrono>

using namespace testing;
using namespace jar;

namespace krn = std::chrono;

namespace {

UvIndexData
getUvIndexData()
{
    CustomData d;
    d.assign({
        {"uv", 15.1},
        {"uvMax", 16.1},
    });
    return UvIndexData{.data = std::move(d)};
}

UvIndexForecastData
getUvIndexForecastData(krn::sys_seconds timestampFrom, krn::sys_seconds timestampTo)
{
    static constexpr auto kStep = krn::hours{3};
    double uv{15.1};
    CustomDataSet dataSet;
    while (timestampFrom < timestampTo) {
        const auto dt = krn::duration_cast<krn::seconds>(timestampFrom.time_since_epoch()).count();
        CustomData d;
        d.assign({
            {"dt", int64_t{dt}},
            {"uv", uv},
            {"duration", int32_t{3600}},
        });
        dataSet.push_back(std::move(d));
        uv += 1.0;
        timestampFrom += kStep;
    }
    return UvIndexForecastData{.data = std::move(dataSet)};
}

} // namespace

class GetUvIndexStatusActionTest : public Test {
public:
    const std::string kIntentName{"test_intent"};

public:
    NiceMock<MockPositioningClient> positioning;
    NiceMock<MockSpeakerClient> speaker;
    NiceMock<MockWeatherClient> weather;
};

TEST_F(GetUvIndexStatusActionTest, GetCurrent)
{
    const auto uvIndexData{getUvIndexData()};
    EXPECT_CALL(speaker, synthesizeSsml(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getUvIndex(_, Not(IsEmpty()), _, _))
        .WillOnce(InvokeArgument<2>(uvIndexData));

    auto action = GetUvStatusAction::create(kIntentName, positioning, speaker, weather);
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    const auto& result = action->result();
    ASSERT_TRUE(result);

    EXPECT_EQ(result->min, 15);
    EXPECT_EQ(result->avg, 15.5);
    EXPECT_EQ(result->max, 16);
}

TEST_F(GetUvIndexStatusActionTest, GetForPeriod)
{
    const krn::sys_seconds t1 = krn::floor<krn::days>(krn::system_clock::now());
    const krn::sys_seconds t2 = t1 + krn::days{1};

    const auto uvIndexForecastData{getUvIndexForecastData(t1, t2)};
    EXPECT_CALL(speaker, synthesizeSsml(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getUvIndexForecast).WillOnce(InvokeArgument<1>(uvIndexForecastData));

    wit::DateTimeEntity entity;
    entity.valueFrom = wit::DateTimeEntity::Value{
        .grain = wit::DateTimeEntity::Grains::hour,
        .timestamp = Timestamp{t1},
    };
    entity.valueTo = wit::DateTimeEntity::Value{
        .grain = wit::DateTimeEntity::Grains::hour,
        .timestamp = Timestamp{t2},
    };

    auto action
        = GetUvStatusAction::create(kIntentName,
                                    positioning,
                                    speaker,
                                    weather,
                                    {
                                        {wit::DateTimeEntity::key(), wit::EntityList{entity}},
                                    });
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    const auto& result = action->result();
    ASSERT_TRUE(result);

    EXPECT_THAT(result->min, Eq(15.0));
    EXPECT_THAT(result->avg, Gt(result->min));
    EXPECT_THAT(result->max, Gt(result->avg));
}