#include "ExecutorApplication.hpp"

#include "common/Config.hpp"
#include "intent/IntentSubsystem.hpp"

#include <jarvis/Logger.hpp>

#include <optional>

namespace jar {

const char*
ExecutorApplication::name() const
{
    return "Executor";
}

const char*
ExecutorApplication::contextId()
{
    return "EXEC";
}

const char*
ExecutorApplication::contextDesc()
{
    return "J.A.R.V.I.S Executor Context";
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
