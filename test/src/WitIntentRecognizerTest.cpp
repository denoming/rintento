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

public:
    net::io_context context;
    WitIntentRecognizer recognizer;
};

TEST_F(WitIntentRecognizerTest, RecognizeMessage)
{
    MockFunction<void(std::string)> callback;
    EXPECT_CALL(callback, Call(Not(IsEmpty()))).WillOnce([](const std::string& result) {
        std::cout << "Result: \n" << result << std::endl;
    });

    std::string_view message{"turn off the light"};
    recognizer.recognize(message, callback.AsStdFunction());

    context.run();
}

TEST_F(WitIntentRecognizerTest, RecognizeSpeech)
{
    MockFunction<void(std::string)> callback;
    EXPECT_CALL(callback, Call(Not(IsEmpty()))).WillOnce([](const std::string& result) {
        std::cout << "Result: \n" << result << std::endl;
    });

    fs::path filePath{"asset/audio/turn-on-the-light.raw"};
    recognizer.recognize(filePath, callback.AsStdFunction());

    context.run();
}
