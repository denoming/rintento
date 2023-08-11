#pragma once

#include <jarvisto/DateTime.hpp>

#include <gmock/gmock.h>

#include "intent/WitTypes.hpp"

namespace testing {

inline Matcher<jar::wit::Intent>
isConfidentIntent(std::string_view name, float threshold)
{
    return AllOf(Field("name", &jar::wit::Intent::name, StrCaseEq(name)),
                 Field("confidence", &jar::wit::Intent::confidence, Gt(threshold)));
}

inline Matcher<const jar::wit::DateTimeEntity&>
matchEntityWithExactTime(jar::wit::DateTimeEntity::Grains grain, std::string_view dateTime)
{
    const auto timestamp = jar::parseZonedDateTime(dateTime);
    return AllOf(
        Field("valueFrom",
              &jar::wit::DateTimeEntity::valueFrom,
              Optional(AllOf(Field(&jar::wit::DateTimeEntity::Value::grain, grain),
                             Field(&jar::wit::DateTimeEntity::Value::timestamp, timestamp)))),
        Field("valueTo",
              &jar::wit::DateTimeEntity::valueTo,
              Optional(AllOf(Field(&jar::wit::DateTimeEntity::Value::grain, grain),
                             Field(&jar::wit::DateTimeEntity::Value::timestamp, timestamp)))));
}

inline Matcher<const jar::wit::DateTimeEntity&>
matchEntityWithTimeRange(jar::wit::DateTimeEntity::Grains grain,
                         std::string_view from,
                         std::string_view to)
{
    return AllOf(Field("valueFrom",
                       &jar::wit::DateTimeEntity::valueFrom,
                       Optional(AllOf(Field(&jar::wit::DateTimeEntity::Value::grain, grain),
                                      Field(&jar::wit::DateTimeEntity::Value::timestamp,
                                            jar::parseZonedDateTime(from))))),
                 Field("valueTo",
                       &jar::wit::DateTimeEntity::valueTo,
                       Optional(AllOf(Field(&jar::wit::DateTimeEntity::Value::grain, grain),
                                      Field(&jar::wit::DateTimeEntity::Value::timestamp,
                                            jar::parseZonedDateTime(to))))));
}

inline Matcher<jar::wit::Utterance>
isUtterance(std::string_view text,
            Matcher<jar::wit::Entities> entities,
            Matcher<jar::wit::Intents> intents,
            const bool final = true)
{
    return AllOf(Field("text", &jar::wit::Utterance::text, StrCaseEq(text)),
                 Field("entities", &jar::wit::Utterance::entities, entities),
                 Field("intents", &jar::wit::Utterance::intents, intents),
                 Field("final", &jar::wit::Utterance::final, final));
}

} // namespace testing
