#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "common/Worker.hpp"
#include "intent/Config.hpp"
#include "intent/WitMessageRecognition.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "test/Matchers.hpp"
#include "test/TestWaiter.hpp"

#include <thread>

using namespace testing;
using namespace jar;

class WitMessageRecognitionTest : public Test {
public:
    WitMessageRecognitionTest()
        : factory{worker.executor()}
        , recognition{factory.message()}
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
    Worker worker;
    WitRecognitionFactory factory;
    TestWaiter waiter;
    std::shared_ptr<WitMessageRecognition> recognition;
};

TEST_F(WitMessageRecognitionTest, RecognizeMessage)
{
    const std::string_view Message{"turn off the light"};

    MockFunction<Recognition::OnReadySignature> callback;
    EXPECT_CALL(callback,
                Call(Contains(isUtterance(Message, Contains(isConfidentIntent("light_off", 0.9f)))),
                     IsFalse()));
    recognition->onReady(callback.AsStdFunction());

    bool guard{false};
    recognition->onData([this, &guard]() {
        guard = true;
        waiter.notify();
    });

    recognition->run();
    EXPECT_FALSE(recognition->ready());

    waiter.wait([&guard]() { return guard; });
    ASSERT_TRUE(guard);
    recognition->feed(Message);

    recognition->wait();
}

TEST_F(WitMessageRecognitionTest, CancelRecognizeMessage)
{
    MockFunction<Recognition::OnReadySignature> callback;
    EXPECT_CALL(callback, Call(IsEmpty(), IsTrue()));
    recognition->onReady(callback.AsStdFunction());
    recognition->run();

    // Waiting some time to simulate real situation
    std::this_thread::sleep_for(std::chrono::milliseconds{50});

    recognition->cancel();
    recognition->wait();
}