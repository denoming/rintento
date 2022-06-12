#include "intent/Uri.hpp"

#include <gtest/gtest.h>

using namespace jar;

TEST(UriTest, RFC2396)
{
    EXPECT_EQ("abcABC123-_.~%21%28%29%26%3d%20", uri::encode("abcABC123-_.~!()&= "))
        << "RFC3986 URL encoded";
    EXPECT_EQ("abcABC123-_.~%21%28%29%26%3d+", uri::encode2("abcABC123-_.~!()&= "))
        << "RFC3986 URL encoded. Space should be escaped to '+'";
    EXPECT_EQ("abcABC123-_.~!()%26%3d%20", uri::encodeUriComponent("abcABC123-_.~!()&= "))
        << "RFC2396 URL encoded";
    EXPECT_EQ("abcABC123-_.~!()%26%3d+", uri::encodeUriComponent2("abcABC123-_.~!()&= "))
        << "RFC2396 URL encoded. Space should be escaped to +";

    EXPECT_EQ("abcABC123-_.~!()&= ", uri::decode("abcABC123-_.~%21%28%29%26%3d%20"))
        << "RFC3986 URL decoded";
    EXPECT_EQ("abcABC123-_.~!()&= ", uri::decode2("abcABC123-_.~%21%28%29%26%3d%20"))
        << "RFC3986 URL decoded";

    EXPECT_EQ("abcABC123-_.~!()&=+", uri::decode("abcABC123-_.~%21%28%29%26%3d+"))
        << "RFC3986 URL decoded. The '+' should be decoded as is";
    EXPECT_EQ("abcABC123-_.~!()&= ", uri::decode2("abcABC123-_.~%21%28%29%26%3d+"))
        << "RFC3986 URL decoded. The '+' should be decoded as space";

    EXPECT_EQ("abcABC123-_.~!()&= ", uri::decode("abcABC123-_.~!()%26%3d%20"))
        << "RFC3986 URL decoded";
    EXPECT_EQ("abcABC123-_.~!()&= ", uri::decode2("abcABC123-_.~!()%26%3d%20"))
        << "RFC3986 URL decoded";

    EXPECT_EQ("abcABC123-_.~!()&=+", uri::decode("abcABC123-_.~!()%26%3d+"))
        << "RFC2396 URL decoded";
    EXPECT_EQ("abcABC123-_.~!()&= ", uri::decode2("abcABC123-_.~!()%26%3d+"))
        << "RFC2396 URL decoded";
}