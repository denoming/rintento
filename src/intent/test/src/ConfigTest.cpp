#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string_view>

#include "intent/Automation.hpp"
#include "intent/Config.hpp"
#include "intent/MockAutomationRegistry.hpp"

using namespace testing;
using namespace jar;

static const std::string_view kConfigValue = R"(
server =
{
    port = 8080;
    threads = 8;
};

wit =
{
    remote =
    {
        host = "api.wit.ai";
        port = "https";
        auth = "Bearer XXXXXX123456789";
    };
};

automations =
(
    {
        alias = "Turn on the light";
        intent = "light_on";
        actions =
        (
            {
                type = "script";
                exec = "program1";
                args = ["-arg1", "-arg2", "value"];
                home = "/path/to/home/directory1";
                env =
                {
                    env1 = "VAR1";
                    env2 = "VAR2";
                };
                inheritParentEnv = true;
                timeout = 3000;
            }
        );
    },
    {
        alias = "Turn off the light";
        intent = "light_off";
        actions =
        (
            {
                type = "script";
                exec = "program2";
                args = ["-arg1", "-arg2", "value"];
                home = "/path/to/home/directory2";
                env =
                {
                    env3 = "VAR3";
                    env4 = "VAR4";
                };
                inheritParentEnv = true;
                timeout = 3000;
            }
        );
    }
);
)";

class ConfigTest : public Test {
public:
    ConfigTest()
        : registry{std::make_shared<MockAutomationRegistry>()}
    {
    }

    MockAutomationRegistry::Ptr registry;
};

TEST_F(ConfigTest, Load)
{
    Config config{registry};

    std::vector<Automation::Ptr> automations;
    EXPECT_CALL(*registry, add).Times(2).WillRepeatedly([&](auto ptr) {
        automations.push_back(std::move(ptr));
    });
    ASSERT_TRUE(config.load(kConfigValue));

    ASSERT_THAT(automations, SizeIs(2));
    EXPECT_THAT(automations[0]->id(), Not(IsEmpty()));
    EXPECT_THAT(automations[0]->alias(), "Turn on the light");
    EXPECT_THAT(automations[0]->intent(), "light_on");
    EXPECT_THAT(automations[1]->id(), Not(IsEmpty()));
    EXPECT_THAT(automations[1]->alias(), "Turn off the light");
    EXPECT_THAT(automations[1]->intent(), "light_off");

    EXPECT_EQ(config.serverPort(), 8080);
    EXPECT_EQ(config.serverThreads(), 8);

    EXPECT_THAT(config.witRemoteHost(), Optional(std::string{"api.wit.ai"}));
    EXPECT_THAT(config.witRemotePort(), Optional(std::string{"https"}));
    EXPECT_THAT(config.witRemoteAuth(), Optional(std::string{"Bearer XXXXXX123456789"}));
}
