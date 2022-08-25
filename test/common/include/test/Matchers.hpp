#pragma once

#include "intent/Types.hpp"

#include <gmock/gmock.h>

namespace testing {

inline Matcher<jar::Intent>
isIntent(std::string_view name)
{
    return Field(&jar::Intent::name, StrCaseEq(name));
}

inline Matcher<jar::Intent>
isIntent(std::string_view name, float confidence)
{
    return AllOf(Field(&jar::Intent::name, StrCaseEq(name)),
                 Field(&jar::Intent::confidence, FloatEq(confidence)));
}

inline Matcher<jar::Intent>
isConfidentIntent(std::string_view name, float threshold)
{
    return AllOf(Field(&jar::Intent::name, StrCaseEq(name)),
                 Field(&jar::Intent::confidence, Gt(threshold)));
}

inline Matcher<jar::Utterance>
isUtterance(std::string_view text, const Matcher<jar::Intents>& intent)
{
    return AllOf(Field(&jar::Utterance::text, StrCaseEq(text)),
                 Field(&jar::Utterance::intents, intent));
}

inline Matcher<jar::Utterance>
isUtterance(std::string_view text)
{
    return Field(&jar::Utterance::text, StrCaseEq(text));
}

} // namespace testing
