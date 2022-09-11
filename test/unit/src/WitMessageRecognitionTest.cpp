#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "common/Config.hpp"
#include "common/Worker.hpp"
#include "intent/WitMessageRecognition.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "test/Matchers.hpp"
#include "test/TestWaiter.hpp"

#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

using namespace testing;
using namespace jar;

static const fs::path kConfigFilePath{fs::current_path() / "asset" / "config" / "config.json"};

class WitMessageRecognitionTest : public Test {
public:
    WitMessageRecognitionTest()
        : factory{config, worker.executor()}
        , recognition{factory.message()}
    {
    }

    static void
    SetUpTestSuite()
    {
        if (!config) {
            config = std::make_shared<Config>();
            ASSERT_TRUE(fs::exists(kConfigFilePath));
            ASSERT_TRUE(config->load(kConfigFilePath));
        }
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
    static std::shared_ptr<Config> config;

    Worker worker;
    WitRecognitionFactory factory;
    TestWaiter waiter;
    std::shared_ptr<WitMessageRecognition> recognition;
};

std::shared_ptr<Config> WitMessageRecognitionTest::config;

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