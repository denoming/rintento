#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/registry/GetUvSafeTimeAction.hpp"
#include "test/MockPositioningClient.hpp"
#include "test/MockSpeakerClient.hpp"
#include "test/MockWeatherClient.hpp"

#include <chrono>

using namespace testing;
using namespace jar;

namespace krn = std::chrono;

namespace {

UvIndexData
getUvSafeTimeData()
{
    CustomData d;
    d.assign({
        {"safe.st1", 150},
    });
    return UvIndexData{.data = std::move(d)};
}

} // namespace

class GetUvSafeTimeActionTest : public Test {
public:
    const std::string kIntentName{"test_intent"};

public:
    NiceMock<MockPositioningClient> positioning;
    NiceMock<MockSpeakerClient> speaker;
    NiceMock<MockWeatherClient> weather;
};

TEST_F(GetUvSafeTimeActionTest, GetUvSafeTime)
{
    const auto uvSafeTimeData{getUvSafeTimeData()};
    EXPECT_CALL(speaker, synthesizeSsml(Not(IsEmpty()), Not(IsEmpty())));
    EXPECT_CALL(weather, getUvIndex(_, Not(IsEmpty()), _, _))
        .WillOnce(InvokeArgument<2>(uvSafeTimeData));

    wit::OrdinaryEntity entity;
    entity.value = 1;

    auto action
        = GetUvSafeTimeAction::create(kIntentName,
                                      positioning,
                                      speaker,
                                      weather,
                                      {
                                          {wit::OrdinaryEntity::key(), wit::EntityList{entity}},
                                      });
    ASSERT_TRUE(action);

    MockFunction<void(std::error_code)> onDone;
    EXPECT_CALL(onDone, Call(std::error_code{}));
    auto c = action->onDone(onDone.AsStdFunction());
    action->perform();
    c.disconnect();

    const auto& result = action->result();
    ASSERT_TRUE(result);

    EXPECT_EQ(result->time, 150);
    EXPECT_EQ(result->skinType, 1);
}