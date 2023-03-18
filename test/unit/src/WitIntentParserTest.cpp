#include "intent/WitIntentParser.hpp"
#include "test/Matchers.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;
using namespace jar;

TEST(WitIntentParserTest, Parse1)
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
    EXPECT_THAT(parser.parse(kInput),
                Optional(Contains(isUtterance(
                    "turn off the light", Contains(isConfidentIntent("light_off", 0.9f)), false))));
}

TEST(WitIntentParserTest, Parse2)
{
    static const std::string_view kInput{R"(
        {
          "text": "Turn"
        }
        {
          "entities": {},
          "intents": [
            {
              "confidence": 0.9709199975944066,
              "id": "695468701564151",
              "name": "light_on"
            }
          ],
          "speech": {
            "confidence": 0,
            "tokens": [
              {
                "confidence": 0.0274,
                "end": 300,
                "start": 0,
                "token": "Turn"
              }
            ]
          },
          "text": "Turn",
          "traits": {}
        }
        {
          "text": "Turn off"
        }
        {
          "text": "Turn off the light"
        }
        {
          "entities": {},
          "intents": [
            {
              "confidence": 0.7909212514264985,
              "id": "554903196208362",
              "name": "light_off"
            }
          ],
          "speech": {
            "confidence": 0,
            "tokens": [
              {
                "confidence": 0.4736,
                "end": 420,
                "start": 0,
                "token": "Turn"
              },
              {
                "confidence": 0.3409,
                "end": 720,
                "start": 420,
                "token": "off"
              }
            ]
          },
          "text": "Turn off",
          "traits": {}
        }
        {
          "entities": {},
          "intents": [
            {
              "confidence": 0.9395968580694023,
              "id": "554903196208362",
              "name": "light_off"
            }
          ],
          "speech": {
            "confidence": 0.9486,
            "tokens": [
              {
                "confidence": 0.9269,
                "end": 420,
                "start": 0,
                "token": "Turn"
              },
              {
                "confidence": 0.9663,
                "end": 720,
                "start": 420,
                "token": "off"
              },
              {
                "confidence": 0.9306,
                "end": 1080,
                "start": 720,
                "token": "the"
              },
              {
                "confidence": 0.9706,
                "end": 1140,
                "start": 1080,
                "token": "light"
              }
            ]
          },
          "text": "Turn off the light",
          "traits": {}
        }
        {
          "entities": {},
          "intents": [
            {
              "confidence": 0.9395968580694023,
              "id": "554903196208362",
              "name": "light_off"
            }
          ],
          "is_final": true,
          "speech": {
            "confidence": 0.9486,
            "tokens": [
              {
                "confidence": 0.9269,
                "end": 420,
                "start": 720,
                "token": "Turn"
              },
              {
                "confidence": 0.9663,
                "end": 720,
                "start": 420,
                "token": "off"
              },
              {
                "confidence": 0.9306,
                "end": 1080,
                "start": 720,
                "token": "the"
              },
              {
                "confidence": 0.9706,
                "end": 1140,
                "start": 1080,
                "token": "light"
              }
            ]
          },
          "text": "Turn off the light",
          "traits": {}
        }
    )"};

    WitIntentParser parser;
    EXPECT_THAT(parser.parse(kInput),
                Optional(Contains(isUtterance("Turn off the light",
                                              Contains(isConfidentIntent("light_off", 0.9f))))));
}

TEST(WitIntentParserTest, ErrorParse)
{
    static const std::string_view kInput{R"(
        {
            "text": "Turn"
        }
        {
            "entities:
    )"};

    WitIntentParser parser;
    EXPECT_EQ(parser.parse(kInput), std::nullopt);
}

TEST(WitIntentParserTest, ParseEmpty)
{
    WitIntentParser parser;
    EXPECT_EQ(parser.parse(""), std::nullopt);
}