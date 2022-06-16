#include "test/Matchers.hpp"
#include "intent/WitIntentParser.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;
using namespace jar;

TEST(WitIntentParserTest, Parse)
{
    static const std::string_view kInput{R"(
        {
          "text": "turn off the light",
          "intents": [
            {
              "id": "554903196208362",
              "name": "light_off",
              "confidence": 0.9265
            }
          ],
          "entities": {},
          "traits": {}
        }
    )"};

    WitIntentParser parser;
    std::error_code ec;
    const auto intents = parser.parse(kInput, ec);
    EXPECT_THAT(intents, Contains(isConfidentIntent("light_off")));
}