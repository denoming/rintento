#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "intent/WitIntentRecognizer.hpp"

using namespace testing;
using namespace jar;

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

TEST_F(WitIntentRecognizerTest, Recognize)
{
    MockFunction<void(std::string)> callback;
    EXPECT_CALL(callback, Call(Not(IsEmpty())));

    recognizer.recognize("turn of the light", callback.AsStdFunction());

    context.run();
}
