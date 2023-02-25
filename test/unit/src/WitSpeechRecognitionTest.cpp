#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "common/Config.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "intent/WitSpeechRecognition.hpp"
#include "jarvis/Worker.hpp"
#include "test/Matchers.hpp"
#include "test/TestWaiter.hpp"
#include "test/Utils.hpp"

#include <fstream>
#include <thread>

using namespace testing;
using namespace jar;

namespace fs = std::filesystem;

class WitSpeechRecognitionTest : public Test {
public:
    const fs::path AssetAudioPath{fs::current_path() / "asset" / "audio"};

    WitSpeechRecognitionTest()
        : factory{config, worker.executor()}
        , recognition{factory.speech()}
    {
    }

    static void
    SetUpTestSuite()
    {
        if (!config) {
            config = std::make_shared<Config>();
            ASSERT_TRUE(config->load());
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
    std::shared_ptr<WitSpeechRecognition> recognition;
};

std::shared_ptr<Config> WitSpeechRecognitionTest::config;

TEST_F(WitSpeechRecognitionTest, RecognizeSpeech1)
{
    const std::string_view Message{"turn off the light"};

    MockFunction<Recognition::OnReady> callback;
    EXPECT_CALL(callback,
                Call(Contains(isUtterance("turn off the light",
                                          Contains(isConfidentIntent("light_off", 0.9f)))),
                     IsFalse()));
    recognition->onReady(callback.AsStdFunction());

    bool guard{false};
    recognition->onData([this, &guard]() {
        guard = true;
        waiter.notify();
    });

    std::size_t fileSize{0};
    auto fileData = readFile(AssetAudioPath / "turn-off-the-light.raw", fileSize);
    ASSERT_TRUE(fileData);
    ASSERT_THAT(fileSize, Gt(0));

    recognition->run();

    waiter.wait([&guard]() { return guard; });
    ASSERT_TRUE(guard);
    guard = false;

    recognition->feed(io::buffer(fileData.get(), fileSize));

    waiter.wait([&guard]() { return guard; });
    ASSERT_TRUE(guard);
    guard = false;

    recognition->finalize();
    recognition->wait();
}

TEST_F(WitSpeechRecognitionTest, RecognizeSpeech2)
{
    MockFunction<Recognition::OnReady> callback;
    EXPECT_CALL(callback,
                Call(Contains(isUtterance("turn on the light",
                                          Contains(isConfidentIntent("light_on", 0.9f)))),
                     IsFalse()));
    recognition->onReady(callback.AsStdFunction());

    bool guard{false};
    recognition->onData([this, &guard]() {
        guard = true;
        waiter.notify();
    });

    std::size_t fileSize{0};
    auto fileData = readFile(AssetAudioPath / "turn-on-the-light.raw", fileSize);
    ASSERT_TRUE(fileData);
    ASSERT_THAT(fileSize, Gt(0));

    recognition->run();

    waiter.wait([&guard]() { return guard; });
    ASSERT_TRUE(guard);
    guard = false;

    recognition->feed(io::buffer(fileData.get(), fileSize));

    waiter.wait([&guard]() { return guard; });
    ASSERT_TRUE(guard);
    guard = false;

    recognition->finalize();
    recognition->wait();
}

TEST_F(WitSpeechRecognitionTest, CancelRecognizeSpeech)
{
    MockFunction<Recognition::OnReady> callback;
    EXPECT_CALL(callback, Call(IsEmpty(), IsTrue()));
    recognition->onReady(callback.AsStdFunction());
    recognition->run();

    // Waiting some time to simulate real situation
    std::this_thread::sleep_for(std::chrono::milliseconds{50});

    recognition->cancel();
    recognition->wait();
}