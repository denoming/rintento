#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/Intent.hpp"
#include "intent/IntentRegistry.hpp"

using namespace testing;
using namespace jar;

class TestIntent final : public Intent {
public:
    TestIntent(std::string name)
        : Intent{std::move(name)}
    {
    }

    std::shared_ptr<Intent>
    clone() final
    {
        return std::make_shared<TestIntent>(name());
    }

    MOCK_METHOD(void, perform, (OnDone), (final));
};

class IntentRegistryTest : public Test {
public:
    const std::string kIntentName{"get_today_rainy_status"};

public:
    IntentRegistry registry;
};

TEST_F(IntentRegistryTest, Add)
{
    EXPECT_FALSE(registry.has(kIntentName));
    registry.add(std::make_shared<TestIntent>(kIntentName));
    EXPECT_TRUE(registry.has(kIntentName));
}

TEST_F(IntentRegistryTest, Remove)
{
    EXPECT_FALSE(registry.has(kIntentName));
    registry.add(std::make_shared<TestIntent>(kIntentName));
    registry.remove(kIntentName);
    EXPECT_FALSE(registry.has(kIntentName));
}
