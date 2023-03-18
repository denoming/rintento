#include "ExecutorApplication.hpp"

#include "common/Config.hpp"
#include "jarvis/Logger.hpp"

#include "intent/IntentSubsystem.hpp"

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
ExecutorApplication::defineOptions(po::options_description& description)
{
    Application::defineOptions(description);
    // clang-format off
    description.add_options()
        ("config-file", po::value<std::string>()->required())
        ;
    // clang-format on
}

void
ExecutorApplication::initialize(Application& application)
{
    std::optional<std::string> configFile;
    if (application.options().contains("config-file")) {
        configFile.emplace(application.options()["config-file"].as<std::string>());
    } else {
        LOGE("No config file path option");
    }

    _config = std::make_shared<Config>();
    if (configFile) {
        if (!_config->load(*configFile)) {
            LOGE("Failed to load config file");
        }
    }

    addSubsystem(std::make_unique<IntentSubsystem>(_config));

    Application::initialize(application);
}

} // namespace jar

APP_MAIN(jar::ExecutorApplication)
