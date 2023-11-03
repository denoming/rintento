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
