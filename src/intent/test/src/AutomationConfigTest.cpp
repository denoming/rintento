#include <gtest/gtest.h>

#include "intent/AutomationConfig.hpp"
#include "intent/MockAutomationRegistry.hpp"

#include <string_view>

using namespace jar;
using namespace testing;

static const std::string_view kConfigValue = R"({
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
                    "timeout": 5000
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
                    "timeout": 3000
                }
            ]
        }
    ]
})";

class AutomationConfigTest : public Test {
public:
    AutomationConfigTest()
        : registry{std::make_shared<MockAutomationRegistry>()}
    {
    }

    MockAutomationRegistry::Ptr registry;
};

TEST_F(AutomationConfigTest, Load)
{
    AutomationConfig config{registry};
    EXPECT_CALL(*registry, add).Times(2);
    ASSERT_TRUE(config.load(kConfigValue));
}