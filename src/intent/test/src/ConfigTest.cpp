// Copyright 2025 Denys Asauliak
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
                type = "mqtt";
                topic = "z2m/hallway/light-switch1/set";
                value = "{\"state\": \"ON\"}";
                host = "192.168.1.43";
                port = 1883;
                user = "denys";
                pass = "123456";
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
