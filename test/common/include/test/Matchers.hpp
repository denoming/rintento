#pragma once

#include <jarvis/DateTime.hpp>

#include <gmock/gmock.h>

#include "intent/WitTypes.hpp"

namespace testing {

inline Matcher<jar::Intent>
isConfidentIntent(std::string_view name, float threshold)
{
    return AllOf(Field("name", &jar::Intent::name, StrCaseEq(name)),
                 Field("confidence", &jar::Intent::confidence, Gt(threshold)));
}

inline Matcher<const jar::DateTimeEntity&>
matchEntityWithExactTime(jar::DateTimeEntity::Grains grain, std::string_view dateTime)
{
    const auto timestamp = jar::parseZonedDateTime(dateTime);
    return AllOf(Field("valueFrom",
                       &jar::DateTimeEntity::valueFrom,
                       Optional(AllOf(Field(&jar::DateTimeEntity::Value::grain, grain),
                                      Field(&jar::DateTimeEntity::Value::timestamp, timestamp)))),
                 Field("valueTo",
                       &jar::DateTimeEntity::valueTo,
                       Optional(AllOf(Field(&jar::DateTimeEntity::Value::grain, grain),
                                      Field(&jar::DateTimeEntity::Value::timestamp, timestamp)))));
}

inline Matcher<const jar::DateTimeEntity&>
matchEntityWithTimeRange(jar::DateTimeEntity::Grains grain,
                         std::string_view from,
                         std::string_view to)
{
    return AllOf(Field("valueFrom",
                       &jar::DateTimeEntity::valueFrom,
                       Optional(AllOf(Field(&jar::DateTimeEntity::Value::grain, grain),
                                      Field(&jar::DateTimeEntity::Value::timestamp,
                                            jar::parseZonedDateTime(from))))),
                 Field("valueTo",
                       &jar::DateTimeEntity::valueTo,
                       Optional(AllOf(Field(&jar::DateTimeEntity::Value::grain, grain),
                                      Field(&jar::DateTimeEntity::Value::timestamp,
                                            jar::parseZonedDateTime(to))))));
}

inline Matcher<jar::Utterance>
isUtterance(std::string_view text,
            Matcher<jar::Entities> entities,
            Matcher<jar::Intents> intents,
            const bool final = true)
{
    return AllOf(Field("text", &jar::Utterance::text, StrCaseEq(text)),
                 Field("entities", &jar::Utterance::entities, entities),
                 Field("intents", &jar::Utterance::intents, intents),
                 Field("final", &jar::Utterance::final, final));
}

} // namespace testing
