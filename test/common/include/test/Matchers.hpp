#pragma once

#include "intent/Types.hpp"

#include <gmock/gmock.h>

namespace testing {

inline Matcher<jar::IntentSpec>
isIntent(std::string_view name)
{
    return Field(&jar::IntentSpec::name, StrCaseEq(name));
}

inline Matcher<jar::IntentSpec>
isIntent(std::string_view name, float confidence)
{
    return AllOf(Field(&jar::IntentSpec::name, StrCaseEq(name)),
                 Field(&jar::IntentSpec::confidence, FloatEq(confidence)));
}

inline Matcher<jar::IntentSpec>
isConfidentIntent(std::string_view name, float threshold)
{
    return AllOf(Field(&jar::IntentSpec::name, StrCaseEq(name)),
                 Field(&jar::IntentSpec::confidence, Gt(threshold)));
}

inline Matcher<jar::UtteranceSpec>
isUtterance(std::string_view text,
            const Matcher<jar::IntentSpecs>& intents,
            const bool isFinal = true)
{
    return AllOf(Field(&jar::UtteranceSpec::text, StrCaseEq(text)),
                 Field(&jar::UtteranceSpec::intents, intents),
                 Field(&jar::UtteranceSpec::final, isFinal));
}

inline Matcher<jar::UtteranceSpec>
isUtterance(std::string_view text)
{
    return Field(&jar::UtteranceSpec::text, StrCaseEq(text));
}

} // namespace testing
