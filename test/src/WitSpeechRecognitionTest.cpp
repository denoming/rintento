#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/Matchers.hpp"
#include "test/Utils.hpp"
#include "tests/TestWorker.hpp"
#include "tests/TestWaiter.hpp"
#include "intent/WitSpeechRecognition.hpp"
#include "intent/WitRecognitionObserver.hpp"

#include <thread>
#include <fstream>

using namespace testing;
using namespace jar;

namespace fs = std::filesystem;

class WitSpeechRecognitionTest : public Test {
public:
    const fs::path AssetAudioPath{fs::current_path() / "asset" / "audio"};

    WitSpeechRecognitionTest()
        : recognition{WitSpeechRecognition::create(worker.sslContext(), worker.executor())}
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
    WitSpeechRecognition::Ptr recognition;
};

TEST_F(WitSpeechRecognitionTest, RecognizeSpeech1)
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

    std::size_t fileSize{0};
    auto fileData = readFile(AssetAudioPath / "turn-off-the-light.raw", fileSize);
    ASSERT_TRUE(fileData);
    ASSERT_THAT(fileSize, Gt(0));

    recognition->run();

    waiter.wait([&guard]() { return guard; });
    ASSERT_TRUE(guard);
    guard = false;

    recognition->feed(net::buffer(fileData.get(), fileSize));

    waiter.wait([&guard]() { return guard; });
    ASSERT_TRUE(guard);
    guard = false;

    recognition->finalize();

    sys::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_FALSE(error);
    EXPECT_THAT(outcome,
                Contains(isUtterance("turn off the light",
                                     Contains(isConfidentIntent("light_off", 0.9f)))));
}

TEST_F(WitSpeechRecognitionTest, RecognizeSpeech2)
{
    MockFunction<WitRecognitionObserver::SuccessSignature> callback;
    EXPECT_CALL(callback, Call(Contains(isUtterance("turn on the light"))));
    auto pending = WitRecognitionObserver::create(recognition);
    ASSERT_TRUE(pending);
    pending->whenSuccess(callback.AsStdFunction());

    bool guard{false};
    signals::scoped_connection c = recognition->onData([this, &guard]() {
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

    recognition->feed(net::buffer(fileData.get(), fileSize));

    waiter.wait([&guard]() { return guard; });
    ASSERT_TRUE(guard);
    guard = false;

    recognition->finalize();

    sys::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_FALSE(error);
    EXPECT_THAT(
        outcome,
        Contains(isUtterance("turn on the light", Contains(isConfidentIntent("light_on", 0.9f)))));
}

TEST_F(WitSpeechRecognitionTest, CancelRecognizeSpeech)
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
    EXPECT_THAT(outcome, IsEmpty());
    EXPECT_EQ(error.value(), int(std::errc::operation_canceled));
}