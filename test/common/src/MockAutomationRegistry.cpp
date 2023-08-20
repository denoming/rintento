#include "test/MockAutomationRegistry.hpp"

using namespace testing;

#include "intent/Automation.hpp"

namespace jar {

MockAutomationRegistry::MockAutomationRegistry()
{
    ON_CALL(*this, has).WillByDefault(Return(true));

    ON_CALL(*this, get).WillByDefault(Return(Automation::Ptr{}));
}

} // namespace jar