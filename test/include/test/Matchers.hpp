#pragma once

#include "intent/Types.hpp"

#include <gmock/gmock.h>

namespace testing {

inline Matcher<jar::Intent>
isIntent(std::string_view name)
{
    return Property(&jar::Intent::name, name);
}

inline Matcher<jar::Intent>
isIntent(std::string_view name, double confidence)
{
    return AllOf(Property(&jar::Intent::name, name),
                 Property(&jar::Intent::confidence, testing::Gt(confidence)));
}

inline testing::Matcher<jar::Intent>
isConfidentIntent(std::string_view name)
{
    return AllOf(Property(&jar::Intent::name, name), Property(&jar::Intent::confident, IsTrue()));
}

} // namespace testing
