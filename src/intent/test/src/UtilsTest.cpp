#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/Utils.hpp"

using namespace jar;
using namespace testing;

TEST(UtilsTest, ParseQueryParams)
{
    static const std::string_view in{"/message?q=turn+on+the+light"};
    EXPECT_THAT(parser::peekMessage(in), Optional(Eq("turn on the light")));
}
