#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/Action.hpp"
#include "intent/ActionRegistry.hpp"

using namespace testing;
using namespace jar;

class TestAction final : public jar::Action {
public:
    TestAction(std::string intent)
        : Action{std::move(intent)}
    {
    }

    std::shared_ptr<Action>
    clone() final
    {
        return std::make_shared<TestAction>(intent());
    }

    MOCK_METHOD(void, perform, (), (final));
};

class ActionRegistryTest : public Test {
public:
    const std::string kIntentName{"test_intent"};

public:
    ActionRegistry registry;
};

TEST_F(ActionRegistryTest, Add)
{
    EXPECT_FALSE(registry.has(kIntentName));
    registry.add(std::make_shared<TestAction>(kIntentName));
    EXPECT_TRUE(registry.has(kIntentName));
}

TEST_F(ActionRegistryTest, Remove)
{
    EXPECT_FALSE(registry.has(kIntentName));
    registry.add(std::make_shared<TestAction>(kIntentName));
    registry.remove(kIntentName);
    EXPECT_FALSE(registry.has(kIntentName));
}
