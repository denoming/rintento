#include "wit/IntentParser.hpp"
#include "wit/Matchers.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;
using namespace jar;

static const std::string_view kMessageResult1{R"({
  "entities": {
    "wit$datetime:datetime": [
      {
        "body": "tomorrow at 1 PM",
        "confidence": 1,
        "end": 40,
        "entities": {},
        "grain": "hour",
        "id": "748915483355230",
        "name": "wit$datetime",
        "role": "datetime",
        "start": 24,
        "type": "value",
        "value": "2023-04-22T13:00:00.000+03:00",
        "values": [
          {
            "grain": "hour",
            "type": "value",
            "value": "2023-04-22T13:00:00.000+03:00"
          }
        ]
      }
    ]
  },
  "intents": [
    {
      "confidence": 0.9977334964561971,
      "id": "981192742889976",
      "name": "get_air_quality_status"
    }
  ],
  "text": "what is air quality for tomorrow at 1 PM",
  "traits": {}
})"};

static const std::string_view kMessageResult2{R"({
  "entities": {
    "wit$datetime:datetime": [
      {
        "body": "today from 1 PM to 4 PM",
        "confidence": 0.9995,
        "end": 43,
        "entities": {},
        "from": {
          "grain": "hour",
          "value": "2023-04-22T13:00:00.000+03:00"
        },
        "id": "748915483355230",
        "name": "wit$datetime",
        "role": "datetime",
        "start": 20,
        "to": {
          "grain": "hour",
          "value": "2023-04-22T17:00:00.000+03:00"
        },
        "type": "interval",
        "values": [
          {
            "from": {
              "grain": "hour",
              "value": "2023-04-22T13:00:00.000+03:00"
            },
            "to": {
              "grain": "hour",
              "value": "2023-04-22T17:00:00.000+03:00"
            },
            "type": "interval"
          }
        ]
      }
    ]
  },
  "intents": [
    {
      "confidence": 0.9992596627830798,
      "id": "981192742889976",
      "name": "get_air_quality_status"
    }
  ],
  "text": "what is air quality today from 1 PM to 4 PM",
  "traits": {}
})"};

static const std::string_view kSpeechInput{R"(
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
    ]},
    "text": "Turn off the light",
    "traits": {}
})"};

TEST(IntentParserTest, ParseMessageResult1)
{
    const auto entries = wit::IntentParser::parseMessageResult(kMessageResult1);
    ASSERT_TRUE(entries.has_value());

    EXPECT_THAT(entries.value(),
                Contains(isUtterance(
                    "what is air quality for tomorrow at 1 PM",
                    Contains(Pair(
                        wit::DateTimeEntity::key(),
                        Contains(VariantWith<wit::DateTimeEntity>(matchEntityWithExactTime(
                            wit::DateTimeEntity::Grains::hour, "2023-04-22T13:00:00.000+03:00"))))),
                    Contains(isConfidentIntent("get_air_quality_status", 0.9f)))));
}

TEST(IntentParserTest, ParseMessageResult2)
{
    const auto entries = wit::IntentParser::parseMessageResult(kMessageResult2);
    ASSERT_TRUE(entries.has_value());

    EXPECT_THAT(entries.value(),
                Contains(isUtterance(
                    "what is air quality today from 1 PM to 4 PM",
                    Contains(Pair(wit::DateTimeEntity::key(),
                                  Contains(VariantWith<wit::DateTimeEntity>(
                                      matchEntityWithTimeRange(wit::DateTimeEntity::Grains::hour,
                                                               "2023-04-22T13:00:00.000+03:00",
                                                               "2023-04-22T17:00:00.000+03:00"))))),
                    Contains(isConfidentIntent("get_air_quality_status", 0.9f)))));
}

TEST(IntentParserTest, ParseSpeechResult)
{
    EXPECT_THAT(
        wit::IntentParser::parseSpeechResult(kSpeechInput),
        Optional(Contains(isUtterance(
            "Turn off the light", IsEmpty(), Contains(isConfidentIntent("light_off", 0.9f))))));
}

TEST(IntentParserTest, ErrorParse)
{
    static const std::string_view kInvalidResult{R"(
        {
            "text": "Turn"
        }
        {
            "entities:
    )"};

    EXPECT_FALSE(wit::IntentParser::parseMessageResult(kInvalidResult));
    EXPECT_FALSE(wit::IntentParser::parseSpeechResult(kInvalidResult));
}

TEST(IntentParserTest, ParseEmpty)
{
    EXPECT_FALSE(wit::IntentParser::parseMessageResult(""));
    EXPECT_FALSE(wit::IntentParser::parseSpeechResult(""));
}