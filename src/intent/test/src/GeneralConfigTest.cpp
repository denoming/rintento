#include <gtest/gtest.h>

#include "intent/GeneralConfig.hpp"

#include <string_view>

using namespace jar;
using namespace testing;

static const std::string_view kConfigValue = R"({
    "server": {
      "port": 8080,
      "threads": 8
    }
})";

TEST(GeneralConfigTest, Load)
{
    GeneralConfig config;
    ASSERT_TRUE(config.load(kConfigValue));

    EXPECT_EQ(config.serverPort(), 8080);
    EXPECT_EQ(config.serverThreads(), 8);
}
