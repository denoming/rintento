#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "intent/WitIntentRecognizer.hpp"

#include <iostream>

using namespace testing;
using namespace jar;

class WitIntentRecognizerTest : public Test {
public:
    void
    SetUp() override
    {
        context.set_default_verify_paths();
        context.set_verify_mode(ssl::verify_peer);
    }

public:
    net::io_context ioc;
    ssl::context context{ssl::context::tlsv12_client};
};

TEST_F(WitIntentRecognizerTest, Recognize)
{
    WitIntentRecognizer recognizer{net::make_strand(ioc), context};
    recognizer.recognize("turn of the light", [](std::string intent) {
        std::cout << intent << std::endl;
    });
    ioc.run();
}
