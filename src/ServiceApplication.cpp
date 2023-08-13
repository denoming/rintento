#include "ServiceApplication.hpp"

#include "common/Config.hpp"
#include "intent/IntentSubsystem.hpp"
#include "rintento/Options.hpp"

#include <jarvisto/Logger.hpp>
#include <jarvisto/LoggerInitializer.hpp>

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

    _config = std::make_shared<Config>();
    if (!_config->load()) {
        LOGE("Unable to load config file");
    }

    addSubsystem(std::make_unique<IntentSubsystem>(_config));

    Application::initialize(application);
}

} // namespace jar

APP_MAIN(jar::ServiceApplication)
