#include <iostream>

#include "cli/Clients.hpp"
#include "common/Constants.hpp"
#include "jarvis/Network.hpp"
#include "jarvis/Worker.hpp"
#include "jarvis/Logger.hpp"
#include "jarvis/LoggerInitializer.hpp"

#include <boost/program_options.hpp>

#include <filesystem>
#include <iostream>

namespace po = boost::program_options;
namespace fs = std::filesystem;

using namespace jar;

int
main(int argn, char* argv[])
{
    LoggerInitializer::instance().initialize();

    std::string message;
    std::string audioFile;
    uint16_t serverPort;

    po::options_description d{"J.A.R.V.I.S Executor CLI"};
    // clang-format off
    d.add_options()
        ("help,h", "Display help")
        ("message,m", po::value<std::string>(&message), "Recognize message")
        ("speech,s", po::value<std::string>(&audioFile), "Recognize speech")
        ("port,p", po::value<uint16_t>(&serverPort)->default_value(kDefaultProxyServerPort), "Recognize server port")
    ;
    // clang-format on

    po::variables_map vm;
    po::store(po::parse_command_line(argn, argv, d), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << d << std::endl;
        return EXIT_SUCCESS;
    }

    Worker worker;
    worker.start();

    if (vm.contains("message")) {
        clients::recognizeMessage(worker.executor(), serverPort, message);
        return EXIT_SUCCESS;
    }

    if (vm.contains("speech")) {
        fs::path audioFilePath{audioFile};
        if (!fs::exists(audioFilePath)) {
            LOGE("File <{}> not found", audioFilePath);
            return EXIT_FAILURE;
        }
        clients::recognizeSpeech(worker.executor(), serverPort, audioFilePath);
        return EXIT_SUCCESS;
    }

    LOGI("Nothing to do. Exit.");
    return EXIT_SUCCESS;
}