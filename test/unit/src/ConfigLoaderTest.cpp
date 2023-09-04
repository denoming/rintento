#include <gtest/gtest.h>

#include "intent/AutomationConfig.hpp"
#include "intent/GeneralConfig.hpp"
#include "test/MockAutomationRegistry.hpp"

#include <jarvisto/Worker.hpp>

#include <string_view>

using namespace jar;
using namespace testing;

static const std::string_view kConfigValue = R"({
    "server": {
      "port": 8080,
      "threads": 8
    },
    "recognition": {
      "server": {
        "host": "api.wit.ai",
        "port": "https",
        "auth": "Bearer 123456789"
      }
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
                    "exec": "program1",
                    "args": ["-arg1", "-arg2", "value"],
                    "home": "/path/to/home/directory1",
                    "env": {
                        "ENV1": "VAR1",
                        "ENV2": "VAR2"
                    },
                    "inheritParentEnv": true,
                    "ttl": 5000
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
                    "exec": "program2",
                    "args": ["-arg1", "-arg2", "value"],
                    "home": "/path/to/home/directory2",
                    "env": {
                        "ENV3": "VAR3",
                        "ENV4": "VAR4"
                    },
                    "inheritParentEnv": true,
                    "ttl": 3000
                }
            ]
        }
    ]
})";

class ConfigLoaderTest : public Test {
public:
    ConfigLoaderTest()
        : registry{std::make_shared<MockAutomationRegistry>()}
    {
    }

    MockAutomationRegistry::Ptr registry;
};

TEST_F(ConfigLoaderTest, GeneralConfig)
{
    GeneralConfig config;
    ASSERT_TRUE(config.load(kConfigValue));

    EXPECT_EQ(config.serverPort(), 8080);
    EXPECT_EQ(config.serverThreads(), 8);
    EXPECT_EQ(config.recognitionServerHost(), "api.wit.ai");
    EXPECT_EQ(config.recognitionServerPort(), "https");
    EXPECT_EQ(config.recognitionServerAuth(), "Bearer 123456789");
}

TEST_F(ConfigLoaderTest, AutomationConfig)
{
    AutomationConfig config{registry};
    EXPECT_CALL(*registry, add).Times(2);
    ASSERT_TRUE(config.load(kConfigValue));
}
