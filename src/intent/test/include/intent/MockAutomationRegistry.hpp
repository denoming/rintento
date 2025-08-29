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

#pragma once

#include "intent/IAutomationRegistry.hpp"

#include <gmock/gmock.h>

namespace jar {

class MockAutomationRegistry : public IAutomationRegistry {
public:
    using Ptr = std::shared_ptr<MockAutomationRegistry>;

    MockAutomationRegistry();

    MOCK_METHOD(void, add, (std::shared_ptr<Automation>), (override));

    MOCK_METHOD(bool, has, (const std::string& intent), (const, override));

    MOCK_METHOD(std::shared_ptr<Automation>, get, (const std::string& intent), (override));
};

} // namespace jar