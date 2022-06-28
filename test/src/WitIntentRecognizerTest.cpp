#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/Matchers.hpp"
#include "intent/WitIntentRecognizer.hpp"

#include <iostream>
#include <filesystem>

using namespace testing;
using namespace jar;

namespace fs = std::filesystem;

class WitIntentRecognizerTest : public Test {
public:
    WitIntentRecognizerTest()
        : recognizer{net::make_strand(context)}
    {
    }

    void
    SetUp() override
    {
        thread = std::thread{[this]() {
            auto guard = net::make_work_guard(context);
            context.run();
        }};
    }

    void
    TearDown() override
    {
        context.stop();
        if (thread.joinable()) {
            thread.join();
        }
    }

public:
    std::thread thread;
    net::io_context context;
    WitIntentRecognizer recognizer;
};

TEST_F(WitIntentRecognizerTest, RecognizeMessage)
{
    MockFunction<void(Utterances result, std::error_code error)> callback;
    EXPECT_CALL(callback, Call(Contains(isUtterance("turn off the light")), IsFalse()));

    std::string_view message{"turn off the light"};
    auto pending = recognizer.recognize(message, callback.AsStdFunction());
    ASSERT_TRUE(pending);

    std::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_FALSE(error);
    EXPECT_THAT(outcome,
                Contains(isUtterance("turn off the light",
                                     Contains(isConfidentIntent("light_off", 0.9f)))));
}

TEST_F(WitIntentRecognizerTest, CancelRecognizeMessage)
{
    MockFunction<void(Utterances result, std::error_code error)> callback;
    EXPECT_CALL(callback, Call(_, IsTrue()));

    std::string_view message{"turn off the light"};
    auto pending = recognizer.recognize(message, callback.AsStdFunction());
    ASSERT_TRUE(pending);

    // Cancel message recognizing
    pending->cancel();

    std::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_THAT(outcome, IsEmpty());
    EXPECT_EQ(error.value(), int(std::errc::operation_canceled));
}

TEST_F(WitIntentRecognizerTest, RecognizeSpeech1)
{
    MockFunction<void(Utterances result, std::error_code error)> callback;
    EXPECT_CALL(callback, Call(Contains(isUtterance("turn off the light")), IsFalse()));

    fs::path filePath{"asset/audio/turn-off-the-light.raw"};
    auto pending = recognizer.recognize(filePath, callback.AsStdFunction());
    ASSERT_TRUE(pending);

    std::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_FALSE(error);
    EXPECT_THAT(outcome,
                Contains(isUtterance("turn off the light",
                                     Contains(isConfidentIntent("light_off", 0.9f)))));
}

TEST_F(WitIntentRecognizerTest, RecognizeSpeech2)
{
    MockFunction<void(Utterances result, std::error_code error)> callback;
    EXPECT_CALL(callback, Call(Contains(isUtterance("turn on the light")), IsFalse()));

    fs::path filePath{"asset/audio/turn-on-the-light.raw"};
    auto pending = recognizer.recognize(filePath, callback.AsStdFunction());
    ASSERT_TRUE(pending);

    std::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_FALSE(error);
    EXPECT_THAT(outcome,
                Contains(isUtterance("turn on the light",
                                     Contains(isConfidentIntent("light_on", 0.9f)))));
}

TEST_F(WitIntentRecognizerTest, CancelRecognizeSpeech)
{
    MockFunction<void(Utterances result, std::error_code error)> callback;
    EXPECT_CALL(callback, Call(_, IsTrue()));

    fs::path filePath{"asset/audio/turn-on-the-light.raw"};
    auto pending = recognizer.recognize(filePath, callback.AsStdFunction());
    ASSERT_TRUE(pending);

    // Cancel message recognizing
    pending->cancel();

    std::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_THAT(outcome, IsEmpty());
    EXPECT_EQ(error.value(), int(std::errc::operation_canceled));
}
