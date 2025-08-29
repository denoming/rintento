// Copyright 2025 Denys Asauliak
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "wit/Utils.hpp"

using namespace jar;
using namespace testing;

#include <regex>

TEST(WitUtilsTest, MessageTarget)
{
    static const std::string_view in{"turn on the light"};
    EXPECT_THAT(wit::messageTarget(in), Eq("/message?q=turn+on+the+light"));
}

TEST(WitUtilsTest, MessageTargetWithDate)
{
    static const std::string_view in{"turn on the light"};
    const auto out = wit::messageTargetWithDate(in);
    static const std::regex re{R"(\/message\?v=\d{8}\&q=turn\+on\+the\+light)"};
    EXPECT_THAT(std::regex_match(out, re), IsTrue());
}

TEST(WitUtilsTest, SpeechTarget)
{
    const auto out = wit::speechTarget();
    EXPECT_THAT(out, Eq("/speech"));
}

TEST(WitUtilsTest, SpeechTargetWithDate)
{
    const auto out = wit::speechTargetWithDate();
    static const std::regex re{R"(\/speech\?v=\d{8})"};
    EXPECT_THAT(std::regex_match(out, re), IsTrue());
}
