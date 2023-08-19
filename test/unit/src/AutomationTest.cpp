#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/Automation.hpp"
#include "intent/SequentActionLaunchStrategy.hpp"
#include "test/Waiter.hpp"

#include <jarvisto/Worker.hpp>

#include <chrono>
#include <thread>

using namespace jar;
using namespace testing;

using namespace std::literals;

class TestAction final : public jar::Action {
public:
    explicit TestAction(std::error_code returnCode = {})
        : _returnCode{returnCode}
    {
    }

    [[nodiscard]] Ptr
    clone() const final
    {
        return Action::Ptr{new TestAction{*this}};
    }

    void
    execute() final
    {
        complete(_returnCode);
    }

private:
    std::error_code _returnCode{};
};

class AutomationTest : public Test {
public:
    void
    SetUp() override
    {
        worker.start();
    }

    void
    TearDown() override
    {
        worker.stop();
    }

public:
    Worker worker;
    SequentActionLaunchStrategy strategy;
};

TEST_F(AutomationTest, Successful)
{
    Waiter<DeferredJob::OnDone> waiter;

    TestAction action1;
    TestAction action2;

    MockFunction<DeferredJob::OnDone> callback;
    EXPECT_CALL(callback, Call(std::error_code{}));

    const auto automation = Automation::create("Test Automation",
                                               "Test Intent",
                                               jar::Action::List{action1.clone(), action2.clone()},
                                               strategy.clone());
    automation->onDone(waiter.enroll([&](const std::error_code ec) { callback.Call(ec); }));
    automation->execute();

    waiter.wait();
}

TEST_F(AutomationTest, Unsuccessful)
{
    Waiter<DeferredJob::OnDone> waiter;

    TestAction action1{std::make_error_code(std::errc::invalid_argument)};
    TestAction action2;

    MockFunction<DeferredJob::OnDone> callback;
    EXPECT_CALL(callback, Call(Not(std::error_code{})));

    const auto automation = Automation::create("Test Automation",
                                               "Test Intent",
                                               jar::Action::List{action1.clone(), action2.clone()},
                                               strategy.clone());
    automation->onDone(waiter.enroll([&](const std::error_code ec) { callback.Call(ec); }));
    automation->execute();

    waiter.wait();
}