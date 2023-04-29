#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/WitHelpers.hpp"

#include <chrono>

using namespace testing;
using namespace jar;

namespace krn = std::chrono;

namespace {

CustomDataSet
getCustomData(krn::sys_seconds timestampFrom, krn::sys_seconds timestampTo, krn::seconds duration)
{
    CustomDataSet output;
    while (timestampFrom <= timestampTo) {
        const auto dt = krn::duration_cast<krn::seconds>(timestampFrom.time_since_epoch()).count();
        CustomData d;
        d.assign({
            {"dt", int64_t{dt}},
            {"duration", static_cast<int32_t>(duration.count())},
        });
        output.push_back(std::move(d));
        timestampFrom += duration;
    }
    return output;
}

CustomDataSet
getCustomData(krn::days modifier, krn::seconds duration)
{
    auto p0 = krn::system_clock::now();
    auto p1 = modifier + krn::floor<krn::days>(p0);
    auto p2 = modifier + krn::ceil<krn::days>(p0);
    return getCustomData(p1, p2, duration);
}

} // namespace

TEST(WitHelperTest, WetherDataPredicate)
{
    const auto data{getCustomData(krn::days{0}, krn::hours{3})};

    const krn::sys_seconds from1 = krn::ceil<krn::days>(krn::system_clock::now());
    const krn::sys_seconds to1 = from1 + krn::hours{2};
    EXPECT_EQ(std::ranges::count_if(data, wit::WeatherDataPredicate{from1, to1}), 1);

    const krn::sys_seconds from2 = krn::floor<krn::days>(krn::system_clock::now());
    const krn::sys_seconds to2 = from2 + krn::hours{5};
    EXPECT_EQ(std::ranges::count_if(data, wit::WeatherDataPredicate{from2, to2}), 2);

    const krn::sys_seconds from3 = krn::floor<krn::days>(krn::system_clock::now()) + krn::hours{2};
    const krn::sys_seconds to3 = from3 + krn::hours{5};
    EXPECT_EQ(std::ranges::count_if(data, wit::WeatherDataPredicate{from3, to3}), 3);
}

TEST(WitHelperTest, EntityPredicate)
{
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

    Entities entities1{
        {DateTimeEntity::key(), EntityList{entity}},
    };
    wit::EntityPredicate<DateTimeEntity> predicate1{entities1};
    EXPECT_TRUE(predicate1.has());
    EXPECT_NO_THROW({ [[maybe_unused]] auto& ref = predicate1.get(); });

    Entities entities2;
    wit::EntityPredicate<DateTimeEntity> predicate2{entities2};
    EXPECT_FALSE(predicate2.has());
    EXPECT_ANY_THROW({ [[maybe_unused]] auto& ref = predicate2.get(); });
}