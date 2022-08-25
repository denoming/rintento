#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/Matchers.hpp"
#include "test/TestWorker.hpp"
#include "test/TestWaiter.hpp"
#include "intent/Config.hpp"
#include "intent/WitMessageRecognition.hpp"
#include "intent/WitRecognitionObserver.hpp"

#include <thread>

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

    MockFunction<WitRecognitionObserver::SuccessSignature> callback;
    EXPECT_CALL(callback, Call(Contains(isUtterance(Message))));
    auto pending = WitRecognitionObserver::create(recognition);
    ASSERT_TRUE(pending);
    pending->whenSuccess(callback.AsStdFunction());

    bool guard{false};
    signals::scoped_connection c = recognition->onData([this, &guard]() {
        guard = true;
        waiter.notify();
    });

    recognition->run();
    EXPECT_FALSE(pending->ready());

    waiter.wait([&guard]() { return guard; });
    ASSERT_TRUE(guard);
    recognition->feed(Message);

    sys::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_TRUE(pending->ready());
    EXPECT_FALSE(error);
    EXPECT_THAT(outcome,
                Contains(isUtterance(Message, Contains(isConfidentIntent("light_off", 0.9f)))));
}

TEST_F(WitMessageRecognitionTest, CancelRecognizeMessage)
{
    MockFunction<WitRecognitionObserver::ErrorSignature> callback;
    EXPECT_CALL(callback, Call(IsTrue()));
    auto pending = WitRecognitionObserver::create(recognition);
    ASSERT_TRUE(pending);
    pending->whenError(callback.AsStdFunction());

    recognition->run();

    // Waiting some time to simulate real situation
    std::this_thread::sleep_for(std::chrono::milliseconds{50});

    pending->cancel();

    sys::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_TRUE(pending->ready());
    EXPECT_EQ(error.value(), int(sys::errc::operation_canceled));
    EXPECT_THAT(outcome, IsEmpty());
}