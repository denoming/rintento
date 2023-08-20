#pragma once

#include "intent/IAutomationRegistry.hpp"

#include <gmock/gmock.h>

namespace jar {

class MockAutomationRegistry : public IAutomationRegistry {
public:
    MockAutomationRegistry();

    MOCK_METHOD(void, add, (std::shared_ptr<Automation>), (override));

    MOCK_METHOD(bool, has, (const std::string& intent), (const, override));

    MOCK_METHOD(std::shared_ptr<Automation>, get, (const std::string& intent), (override));
};

} // namespace jar