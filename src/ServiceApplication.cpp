#include "ServiceApplication.hpp"

#include "common/Config.hpp"
#include "intent/IntentSubsystem.hpp"

#include <jarvisto/Logger.hpp>

namespace jar {

const char*
ServiceApplication::name() const
{
    return "Rintento";
}

void
ServiceApplication::initialize(Application& application)
{
    _config = std::make_shared<Config>();

    if (!_config->load()) {
        LOGE("Failed to load config file");
    }

    addSubsystem(std::make_unique<IntentSubsystem>(_config));

    Application::initialize(application);
}

} // namespace jar

APP_MAIN(jar::ServiceApplication)
