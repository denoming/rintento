#include "ServiceApplication.hpp"

#include "intent/IntentSubsystem.hpp"
#include "rintento/Options.hpp"

#include <jarvisto/core/LoggerInitializer.hpp>

namespace jar {

const char*
ServiceApplication::name() const
{
    return "Rintento";
}

void
ServiceApplication::initialize(Application& application)
{
#ifdef ENABLE_DLT
    LoggerInitializer::instance().initialize("RINT", name(), "MAIN", "Main Context");
#else
    LoggerInitializer::instance().initialize();
#endif

    addSubsystem(std::make_unique<IntentSubsystem>());

    Application::initialize(application);
}

} // namespace jar

APP_MAIN(jar::ServiceApplication)
