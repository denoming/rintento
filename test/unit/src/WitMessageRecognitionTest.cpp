#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/GeneralConfig.hpp"
#include "intent/WitMessageRecognition.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "test/Matchers.hpp"
#include "test/TestWaiter.hpp"

#include <jarvisto/Worker.hpp>

#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

using namespace testing;
using namespace jar;

class WitMessageRecognitionTest : public Test {
public:
    WitMessageRecognitionTest()
        : factory{config.recognizeServerHost(),
                  config.recognizeServerPort(),
                  config.recognizeServerAuth(),
                  worker.executor()}
        , recognition{factory.message()}
    {
    }

    static void
    SetUpTestSuite()
    {
        ASSERT_TRUE(config.load());
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
    static GeneralConfig config;

    Worker worker;
    WitRecognitionFactory factory;
    TestWaiter waiter;
    std::shared_ptr<WitMessageRecognition> recognition;
};

GeneralConfig WitMessageRecognitionTest::config;

TEST_F(WitMessageRecognitionTest, RecognizeMessage)
{
    const std::string_view Message{"turn off the light"};

    MockFunction<WitRecognition::OnDone> callback;
    EXPECT_CALL(callback,
                Call(Contains(isUtterance(
                         Message, IsEmpty(), Contains(isConfidentIntent("light_off", 0.9f)))),
                     IsFalse()));
    recognition->onDone(callback.AsStdFunction());

    bool guard{false};
    recognition->onData([this, &guard]() {
        guard = true;
        waiter.notify();
    });

    recognition->run();
    EXPECT_FALSE(recognition->needData());

    waiter.wait([&guard]() { return guard; });
    ASSERT_TRUE(guard);
    recognition->feed(Message);

    recognition->wait();
}

TEST_F(WitMessageRecognitionTest, CancelRecognizeMessage)
{
    MockFunction<WitRecognition::OnDone> callback;
    EXPECT_CALL(callback, Call(IsEmpty(), IsTrue()));
    recognition->onDone(callback.AsStdFunction());
    recognition->run();

    // Waiting some time to simulate real situation
    std::this_thread::sleep_for(std::chrono::milliseconds{50});

    recognition->cancel();
    recognition->wait();
}