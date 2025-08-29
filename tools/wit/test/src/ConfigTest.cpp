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
