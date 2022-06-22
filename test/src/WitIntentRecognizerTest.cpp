#include <gtest/gtest.h>
#include <gmock/gmock.h>

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
    std::string_view message{"turn off the light"};
    auto pending = recognizer.recognize(message);
    ASSERT_TRUE(pending);

    std::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_FALSE(error);
    EXPECT_THAT(outcome, Not(IsEmpty()));
}

TEST_F(WitIntentRecognizerTest, RecognizeSpeech)
{
    fs::path filePath{"asset/audio/turn-on-the-light.raw"};
    auto pending = recognizer.recognize(filePath);

    std::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_FALSE(error);
    EXPECT_THAT(outcome, Not(IsEmpty()));
}
