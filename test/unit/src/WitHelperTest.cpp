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

    const auto now = krn::system_clock::now();
    const auto t1 = krn::ceil<krn::days>(now);
    const auto t2 = t1 + krn::hours{2};
    EXPECT_EQ(std::ranges::count_if(data, wit::DateTimePredicate{Timestamp{t1}, Timestamp{t2}}), 1);

    const auto t3 = krn::floor<krn::days>(now);
    const auto t4 = t3 + krn::hours{5};
    EXPECT_EQ(std::ranges::count_if(data, wit::DateTimePredicate{Timestamp{t3}, Timestamp{t4}}), 2);

    const auto t5 = krn::floor<krn::days>(now) + krn::hours{2};
    const auto t6 = t5 + krn::hours{5};
    EXPECT_EQ(std::ranges::count_if(data, wit::DateTimePredicate{Timestamp{t5}, Timestamp{t6}}), 3);
}

TEST(WitHelperTest, EntityPredicate)
{
    const auto now = krn::ceil<krn::days>(krn::system_clock::now());
    const auto ts1 = Timestamp{now + krn::hours{1}};
    const auto ts2 = Timestamp{now + krn::hours{3}};

    DateTimeEntity entity;
    entity.valueFrom = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = ts1,
    };
    entity.valueTo = DateTimeEntity::Value{
        .grain = DateTimeEntity::Grains::hour,
        .timestamp = ts2,
    };

    Entities entities1{
        {DateTimeEntity::key(), EntityList{entity}},
    };
    wit::EntityGetter<DateTimeEntity> predicate1{entities1};
    EXPECT_TRUE(predicate1.has());
    EXPECT_NO_THROW({ [[maybe_unused]] auto& ref = predicate1.get(); });

    Entities entities2;
    wit::EntityGetter<DateTimeEntity> predicate2{entities2};
    EXPECT_FALSE(predicate2.has());
    EXPECT_ANY_THROW({ [[maybe_unused]] auto& ref = predicate2.get(); });
}