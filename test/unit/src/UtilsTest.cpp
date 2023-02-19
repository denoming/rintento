#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/Utils.hpp"

using namespace jar;
using namespace testing;

#include <regex>

TEST(UtilsTest, MessageTarget)
{
    static const std::string_view in{"turn on the light"};
    EXPECT_THAT(format::messageTarget(in), Eq("/message?q=turn+on+the+light"));
}

TEST(UtilsTest, MessageTargetWithDate)
{
    static const std::string_view in{"turn on the light"};
    const auto out = format::messageTargetWithDate(in);
    static const std::regex re{R"(\/message\?v=\d{8}\&q=turn\+on\+the\+light)"};
    EXPECT_THAT(std::regex_match(out, re), IsTrue());
}

TEST(UtilsTest, SpeechTarget)
{
    const auto out = format::speechTarget();
    EXPECT_THAT(out, Eq("/speech"));
}

TEST(UtilsTest, SpeechTargetWithDate)
{
    const auto out = format::speechTargetWithDate();
    static const std::regex re{R"(\/speech\?v=\d{8})"};
    EXPECT_THAT(std::regex_match(out, re), IsTrue());
}

TEST(UtilsTest, ParseQueryParams)
{
    static const std::string_view in{"/message?q=turn+on+the+light"};
    EXPECT_THAT(parser::peekMessage(in), Optional(Eq("turn on the light")));
}
