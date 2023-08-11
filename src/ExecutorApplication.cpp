#include "ExecutorApplication.hpp"

#include "common/Config.hpp"
#include "intent/IntentSubsystem.hpp"

#include <jarvisto/Logger.hpp>

#include <optional>

namespace jar {

const char*
ExecutorApplication::name() const
{
    return "Executor";
}

void
ExecutorApplication::initialize(Application& application)
{
    _config = std::make_shared<Config>();

    if (!_config->load()) {
        LOGE("Failed to load config file");
    }

    addSubsystem(std::make_unique<IntentSubsystem>(_config));

    Application::initialize(application);
}

} // namespace jar

APP_MAIN(jar::ExecutorApplication)
