#include "ExecutorService.hpp"

#include "common/Config.hpp"
#include "common/Logger.hpp"
#include "common/LoggerInitializer.hpp"

#include "intent/IntentSubsystem.hpp"

#include <optional>

namespace jar {

const char*
ExecutorService::name() const
{
    return "ExecutorService";
}

void
ExecutorService::defineOptions(po::options_description& description)
{
    Application::defineOptions(description);
    // clang-format off
    description.add_options()
        ("config-file", po::value<std::string>()->required())
        ;
    // clang-format on
}

void
ExecutorService::initialize(Application& application)
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

void
ExecutorService::proceed()
{
    if (!waitForTermination()) {
        LOGE("Waiting for termination has failed");
    }
}

} // namespace jar

APP_MAIN(jar::ExecutorService)
