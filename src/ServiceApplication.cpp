#include "ServiceApplication.hpp"

#include "common/ServiceLogger.hpp"
#include "intent/IntentSubsystem.hpp"

namespace jar {

const char*
ServiceApplication::name() const
{
    return "Rintento";
}

void
ServiceApplication::initialize(Application& application)
{
    ServiceLogger logger{"RINTENTO"};
    logger.create("MAIN", SPDLOG_LEVEL_DEBUG);

    addSubsystem(std::make_unique<IntentSubsystem>());

    Application::initialize(application);
}

} // namespace jar

APP_MAIN(jar::ServiceApplication)
