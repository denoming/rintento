#include "common/Subsystem.hpp"

#include "common/Logger.hpp"

namespace jar {

void
Subsystem::initialize(Application& /*application*/)
{
    LOGI("Initialize <{}> subsystem", name());
}

void
Subsystem::setUp(Application& /*application*/)
{
    LOGI("Set up <{}> subsystem", name());
}

void
Subsystem::tearDown()
{
    LOGI("Tear down <{}> subsystem", name());
}

void
Subsystem::finalize()
{
    LOGI("Finalize <{}> subsystem", name());
}

} // namespace jar