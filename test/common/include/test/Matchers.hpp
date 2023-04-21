#pragma once

#include "intent/Types.hpp"

#include <gmock/gmock.h>

namespace testing {

inline Matcher<jar::Intent>
isConfidentIntent(std::string_view name, float threshold)
{
    return AllOf(Field("name", &jar::Intent::name, StrCaseEq(name)),
                 Field("confidence", &jar::Intent::confidence, Gt(threshold)));
}

inline Matcher<jar::Utterance>
isUtterance(std::string_view text,
            const Matcher<jar::Intents>& intents,
            const bool final = true)
{
    return AllOf(Field("text", &jar::Utterance::text, StrCaseEq(text)),
                 Field("intents", &jar::Utterance::intents, intents),
                 Field("final", &jar::Utterance::final, final));
}

} // namespace testing
