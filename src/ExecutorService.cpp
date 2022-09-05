#include "ExecutorService.hpp"

#include "common/Logger.hpp"
#include "common/LoggerInitializer.hpp"

#include "intent/IntentSubsystem.hpp"

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

    description.add_options()("port,p", po::value<int>(), "The number of port");
}

void
ExecutorService::initialize(Application& application)
{
    addSubsystem(std::make_unique<IntentSubsystem>());

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

int
main(int argn, char* argv[])
{
    try {
        jar::LoggerInitializer::initialize();
        jar::ExecutorService service;
        service.parseArgs(argn, argv);
        service.run();
    } catch (std::exception& e) {
        LOGE("Unhandled exception: {}", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        LOGE("Unknown exception");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
