#include <gtest/gtest.h>

#include "intent/AutomationConfig.hpp"
#include "intent/GeneralConfig.hpp"
#include "test/MockAutomationRegistry.hpp"

#include <jarvisto/Worker.hpp>

using namespace jar;
using namespace testing;

#include <string_view>

static const std::string_view kConfigValue = R"({
    "proxy":
    {
        "port": 8000,
        "threads": 2
    },
    "recognize":
    {
        "host": "api.wit.ai",
        "port": "https",
        "auth": "Bearer 123456789",
        "threads": 4
    },
    "automations":
    [
        {
            "alias": "",
            "intent": "light_on",
            "actions":
            [
                {
                    "type": "script",
                    "command": "/path/to/execute1 arg1 arg2",
                    "environment":
                    {
                        "ENV1": "1",
                        "ENV2": "2"
                    }
                }
            ]
        },
        {
            "alias": "",
            "intent": "light_off",
            "actions":
            [
                {
                    "type": "script",
                    "command": "/path/to/execute2 arg1 arg2",
                    "environment":
                    {
                        "ENV3": "3",
                        "ENV4": "4"
                    }
                }
            ]
        }
    ]
})";

class ConfigLoaderTest : public Test {
public:
    MockAutomationRegistry registry;
};

TEST_F(ConfigLoaderTest, GeneralConfig)
{
    GeneralConfig config;
    ASSERT_TRUE(config.load(kConfigValue));

    EXPECT_EQ(config.proxyServerPort(), 8000);
    EXPECT_EQ(config.proxyServerThreads(), 2);
    EXPECT_EQ(config.recognizeServerHost(), "api.wit.ai");
    EXPECT_EQ(config.recognizeServerPort(), "https");
    EXPECT_EQ(config.recognizeServerAuth(), "Bearer 123456789");
    EXPECT_EQ(config.recognizeThreads(), 4);
}

TEST_F(ConfigLoaderTest, AutomationConfig)
{
    Worker worker;
    AutomationConfig config{worker, registry};
    EXPECT_CALL(registry, add).Times(2);
    ASSERT_TRUE(config.load(kConfigValue));
}