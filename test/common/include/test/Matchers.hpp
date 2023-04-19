#pragma once

#include "intent/Types.hpp"

#include <gmock/gmock.h>

namespace testing {

inline Matcher<jar::IntentSpec>
isConfidentIntent(std::string_view name, float threshold)
{
    return AllOf(Field("name", &jar::IntentSpec::name, StrCaseEq(name)),
                 Field("confidence", &jar::IntentSpec::confidence, Gt(threshold)));
}

inline Matcher<jar::UtteranceSpec>
isUtterance(std::string_view text,
            const Matcher<jar::IntentSpecs>& intents,
            const bool final = true)
{
    return AllOf(Field("text", &jar::UtteranceSpec::text, StrCaseEq(text)),
                 Field("intents", &jar::UtteranceSpec::intents, intents),
                 Field("final", &jar::UtteranceSpec::final, final));
}

} // namespace testing
