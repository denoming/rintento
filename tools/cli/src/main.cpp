#include <iostream>

#include "cli/Clients.hpp"
#include "common/Constants.hpp"
#include "jarvis/Network.hpp"
#include "jarvis/Worker.hpp"

#include <boost/program_options.hpp>

#include <filesystem>

namespace po = boost::program_options;
namespace fs = std::filesystem;

using namespace jar;

int
main(int argn, char* argv[])
{
    std::string message;
    std::string file;
    uint16_t serverPort;

    po::options_description d{"J.A.R.V.I.S Executor CLI"};
    // clang-format off
    d.add_options()
        ("help,h", "Display help")
        ("message,m", po::value<std::string>(&message), "Recognize message")
        ("speech,s", po::value<std::string>(&file), "Recognize speech")
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
        fs::path filePath{file};
        if (!fs::exists(filePath)) {
            std::cerr << "File: '" << filePath << "' not found" << std::endl;
            return EXIT_FAILURE;
        }
        clients::recognizeSpeech(worker.executor(), serverPort, filePath);
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}