#include "common/Application.hpp"

#include "common/Logger.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>

#include <iostream>

namespace asio = boost::asio;
namespace sys = boost::system;

namespace jar {

Application::Application()
    : _helpRequested{false}
{
}

void
Application::parseArgs(int argc, char* argv[])
{
    processOptions(argc, argv);
}

int
Application::run()
{
    if (_helpRequested) {
        return EXIT_SUCCESS;
    }

    try {
        initialize(*this);
        setUp(*this);
        proceed();
        tearDown();
    } catch (const std::exception& e) {
        LOGE("Application exception: {}", e.what());
        return EXIT_FAILURE;
    }
    finalize();
    return EXIT_SUCCESS;
}

const po::variables_map&
Application::options() const
{
    return _options;
}

void
Application::defineOptions(po::options_description& description)
{
    description.add_options()("help,h", "Display help");
}

bool
Application::waitForTermination()
{
    asio::io_context context;
    boost::asio::signal_set signals(context, SIGINT, SIGTERM);
    signals.async_wait([&context](const sys::error_code& error, int signal) {
        if (!context.stopped()) {
            context.stop();
        }
    });
    return (context.run() > 0);
}

void
Application::addSubsystem(Subsystem::Ptr subsystem)
{
    _subsystems.push_back(std::move(subsystem));
}

void
Application::initialize(Application& application)
{
    for (auto& subsystem : _subsystems) {
        subsystem->initialize(*this);
    }
}

void
Application::setUp(Application& application)
{
    for (auto& subsystem : _subsystems) {
        subsystem->setUp(*this);
    }
}

void
Application::tearDown()
{
    for (auto& subsystem : _subsystems) {
        subsystem->tearDown();
    }
}

void
Application::finalize()
{
    for (auto& subsystem : _subsystems) {
        subsystem->finalize();
    }
}

void
Application::processOptions(int argc, char* argv[])
{
    po::options_description d{"Options"};
    defineOptions(d);
    po::store(po::parse_command_line(argc, argv, d), _options);
    if (_options.contains("help")) {
        handleHelp(d);
    }
}

void
Application::handleHelp(const po::options_description& description)
{
    _helpRequested = true;

    std::cout << description << std::endl;
}

} // namespace jar