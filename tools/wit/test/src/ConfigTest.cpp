#include <gtest/gtest.h>

#include "wit/Config.hpp"

#include <string_view>

using namespace jar;
using namespace testing;

static const std::string_view kConfigValue = R"(
wit =
{
    remote =
    {
        host = "api.wit.ai";
        port = "https";
        auth = "Bearer 123456789";
    };
};
)";

TEST(WitConfigTest, Load)
{
    wit::Config config;
    ASSERT_TRUE(config.load(kConfigValue));

    EXPECT_EQ(config.remoteHost(), "api.wit.ai");
    EXPECT_EQ(config.remotePort(), "https");
    EXPECT_EQ(config.remoteAuth(), "Bearer 123456789");
}
