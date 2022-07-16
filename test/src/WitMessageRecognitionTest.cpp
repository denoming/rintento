#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/Matchers.hpp"
#include "tests/TestWorker.hpp"
#include "tests/TestWaiter.hpp"
#include "intent/Config.hpp"
#include "intent/WitMessageRecognition.hpp"
#include "intent/WitRecognitionObserver.hpp"

using namespace testing;
using namespace jar;

class WitMessageRecognitionTest : public Test {
public:
    WitMessageRecognitionTest()
        : recognition{WitMessageRecognition::create(worker.sslContext(), worker.executor())}
    {
    }

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
    TestWorker worker;
    TestWaiter waiter;
    WitMessageRecognition::Ptr recognition;
};

TEST_F(WitMessageRecognitionTest, RecognizeMessage)
{
    const std::string_view Message{"turn off the light"};

    MockFunction<WitRecognitionObserver::CallbackSignature> callback;
    EXPECT_CALL(callback, Call(Contains(isUtterance(Message)), IsFalse()));
    auto pending = WitRecognitionObserver::create(recognition, callback.AsStdFunction());

    bool guard{false};
    auto c = recognition->onData([this, &guard]() {
        guard = true;
        waiter.notify();
    });

    recognition->run();
    waiter.wait([&guard]() { return guard; });
    recognition->feed(Message);

    std::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_FALSE(error);
    EXPECT_THAT(outcome,
                Contains(isUtterance(Message, Contains(isConfidentIntent("light_off", 0.9f)))));

    c.disconnect();
}

TEST_F(WitMessageRecognitionTest, CancelRecognizeMessage)
{
    MockFunction<WitRecognitionObserver::CallbackSignature> callback;
    EXPECT_CALL(callback, Call(_, IsTrue()));
    auto pending = WitRecognitionObserver::create(recognition, callback.AsStdFunction());

    recognition->run();

    pending->cancel();

    std::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_EQ(error.value(), int(std::errc::operation_canceled));
    EXPECT_THAT(outcome, IsEmpty());
}