#pragma once

#include "intent/Types.hpp"

#include <gmock/gmock.h>

namespace testing {

inline Matcher<jar::Intent>
isIntent(const std::string& name)
{
    return Field(&jar::Intent::name, StrCaseEq(name));
}

inline Matcher<jar::Intent>
isIntent(const std::string& name, float confidence)
{
    return AllOf(Field(&jar::Intent::name, StrCaseEq(name)),
                 Field(&jar::Intent::confidence, FloatEq(confidence)));
}

inline Matcher<jar::Intent>
isConfidentIntent(const std::string& name, float threshold)
{
    return AllOf(Field(&jar::Intent::name, StrCaseEq(name)),
                 Field(&jar::Intent::confidence, Gt(threshold)));
}

inline Matcher<jar::Utterance>
isUtterance(const std::string& text, const Matcher<jar::Intents>& intent)
{
    return AllOf(Field(&jar::Utterance::text, StrCaseEq(text)),
                 Field(&jar::Utterance::intents, intent));
}

inline Matcher<jar::Utterance>
isUtterance(const std::string& text)
{
    return Field(&jar::Utterance::text, StrCaseEq(text));
}

} // namespace testing
