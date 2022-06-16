#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/Matchers.hpp"
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
    MockFunction<void(Intents)> callback;
    EXPECT_CALL(callback, Call(Contains(isConfidentIntent("light_off"))));
    recognizer.recognize("turn off the light", callback.AsStdFunction());
    context.run();
}
